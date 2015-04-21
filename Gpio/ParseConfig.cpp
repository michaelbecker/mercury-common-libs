#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "GpioPrivate.h"
#include "LogLib.h"



void __attribute__  ((visibility ("default")))
TaGpioPrintConfig(void)
{
    int i;

    printf("GPIO Inputs\n");
    printf("------------------------------------\n");
    for (i = 0; i<NUM_GPIO_INPUTS; i++){
        GpioInputs[i].ToString();
    }

    printf("GPIO Outputs\n");
    printf("------------------------------------\n");
    for (i = 0; i<NUM_GPIO_OUTPUTS; i++){
        GpioOutputs[i].ToString();
    }
}



void __attribute__  ((visibility ("default")))
TaGpioLogConfig(void)
{
    int i;

    for (i = 0; i<NUM_GPIO_INPUTS; i++){
        GpioInputs[i].LogString();
    }

    for (i = 0; i<NUM_GPIO_OUTPUTS; i++){
        GpioOutputs[i].LogString();
    }
}



static int 
GetGpioIndexFromName(char *tag)
{
    if (strcmp(tag, "EXCHANGER_FAULT") == 0){
        return GPIO_IN_EXCHANGER_FAULT;
    }
    else if (strcmp(tag, "FAULT_1") == 0){
        return GPIO_IN_FAULT_1;
    }
    else if (strcmp(tag, "FAULT_2") == 0){
        return GPIO_IN_FAULT_2;
    }
    else if (strcmp(tag, "HEATER_FAULT") == 0){
        return GPIO_IN_HEATER_FAULT;
    }
    else if (strcmp(tag, "SYNC") == 0){
        return GPIO_IN_SYNC;
    }
    else if (strcmp(tag, "NO_CAN_PWR") == 0){
        return GPIO_IN_NO_CAN_POWER;
    }
    else if (strcmp(tag, "UART1_DTR") == 0){
        return GPIO_IN_UART1_DTR;
    }
    else if (strcmp(tag, "EXCHANGER_OFF") == 0){
        return GPIO_OUT_EXCHANGER_OFF;
    }
    else if (strcmp(tag, "HEATER_OFF") == 0){
        return GPIO_OUT_HEATER_OFF;
    }
    else if (strcmp(tag, "EVENT") == 0){
        return GPIO_OUT_EVENT;
    }
    else if (strcmp(tag, "GAS") == 0){
        return GPIO_OUT_GAS;
    }
    else if (strcmp(tag, "RED_LED") == 0){
        return GPIO_OUT_RED_LED;
    }
    else{
        return -1;
    }
}



static int 
ParseInputConfingLine(char *line, ssize_t num_chars_read)
{
    char *Tag;
    char *Io;
    char *ActiveLow;
    int index = 0;

    Tag = &line[0];
    while (!isspace(line[index])){
        if (++index >= num_chars_read) {
            return -1;
        }
    }
    while (isspace(line[index])){
        line[index] = 0;
        if (++index >= num_chars_read) {
            return -1;
        }
    }

    Io=&line[index];
    while (!isspace(line[index])){
        if (++index >= num_chars_read) {
            return -1;
        }
    }
    while (isspace(line[index])){
        line[index] = 0;
        if (++index >= num_chars_read) {
            return -1;
        }
    }

    ActiveLow=&line[index];
    while (!isspace(line[index])){
        if (++index >= num_chars_read) {
            return -1;
        }
    }
    while (isspace(line[index])){
        line[index] = 0;
        if (++index >= num_chars_read) {
            break;
        }
    }

    index = GetGpioIndexFromName(Tag);
    if (index == -1){
        return -1;
    }
    
    GpioInputs[index].Init( Io, 
                            ActiveLow, 
                            Tag);

    return 0;
}



static int 
ParseOutputConfingLine(char *line, ssize_t num_chars_read)
{
    char *Tag;
    char *Io;
    char *ActiveLow;
    char *DefaultValue;
    int index = 0;

    Tag = &line[0];
    while (!isspace(line[index])){
        if (++index >= num_chars_read) {
            return -1;
        }
    }
    while (isspace(line[index])){
        line[index] = 0;
        if (++index >= num_chars_read) {
            return -1;
        }
    }

    Io=&line[index];
    while (!isspace(line[index])){
        if (++index >= num_chars_read) {
            return -1;
        }
    }
    while (isspace(line[index])){
        line[index] = 0;
        if (++index >= num_chars_read) {
            return -1;
        }
    }

    ActiveLow=&line[index];
    while (!isspace(line[index])){
        if (++index >= num_chars_read) {
            return -1;
        }
    }
    while (isspace(line[index])){
        line[index] = 0;
        if (++index >= num_chars_read) {
            return -1;
        }
    }

    DefaultValue=&line[index];
    while (!isspace(line[index])){
        if (++index >= num_chars_read) {
            return -1;
        }
    }
    while (isspace(line[index])){
        line[index] = 0;
        if (++index >= num_chars_read) {
            break;
        }
    }

    index = GetGpioIndexFromName(Tag);
    if (index == -1){
        return -1;
    }

    GpioOutputs[index].Init(Io, 
                            ActiveLow, 
                            Tag, 
                            DefaultValue);
    
    return 0;
}



int 
ParseConfigFile(char *ConfigFile)
{
    //-------------------------------
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t num_chars_read;
    enum {
        InitState,
        ParseInputsState,
        ParseOutputsState,
    }State = InitState;
    int rc = 0;
    //-------------------------------


    fp = fopen(ConfigFile, "r");
    if (fp == NULL){
        LogMessage("GPIO FAILED OPENING %s", ConfigFile);
        return -1;
    }

    while ((num_chars_read = getline(&line, &len, fp)) != -1){
        if (line[0] == '#'){
            //  
            //  Skip comments
            //
        }
        else if ((line[0] == ' ') || (line[0] == '\t') || (line[0] == '\n')){
            //  
            //  Skip blank lines
            //
        }
        else if (strstr(line, "INPUTS") != NULL){
            State = ParseInputsState;
        }
        else if (strstr(line, "OUTPUTS") != NULL){
            State = ParseOutputsState;
        }
        else if (State == ParseInputsState){
            char *DupLine = strdup(line);
            if ((rc = ParseInputConfingLine(line, num_chars_read)) == -1){
                LogMessage("GPIO INPUT PARSE FAILED (%s): %s", ConfigFile, DupLine);
            }
            free(DupLine);
        }
        else if (State == ParseOutputsState){
            char *DupLine = strdup(line);
            if ((rc = ParseOutputConfingLine(line, num_chars_read)) == -1){
                LogMessage("GPIO OUTPUT PARSE FAILED (%s): %s", ConfigFile, DupLine);
            }
            free(DupLine);
        }
        else{
            LogMessage("GPIO STATEMENT UNKNOWN (%s): %s", ConfigFile, line);
        }
    }

    free(line);

    return 0;
}



