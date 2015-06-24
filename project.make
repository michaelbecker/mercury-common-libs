###############################################################################
#
#	Makefile
#
###############################################################################


default:
	for dir in ${NODE_DIRS};        \
        do ${MAKE} -C $$dir;        \
        rc=$$?;                     \
        if [ $$rc != 0 ]; then      \
            echo fail $$rc;         \
            exit 1;                 \
        fi;                         \
    done


all:
	for dir in ${NODE_DIRS};        \
        do ${MAKE} -C $$dir all;    \
        rc=$$?;                     \
        if [ $$rc != 0 ]; then      \
            echo fail $$rc;         \
            exit 1;                 \
        fi;                         \
    done


clean:
	for dir in ${NODE_DIRS};        \
        do ${MAKE} -C $$dir clean;  \
        rc=$$?;                     \
        if [ $$rc != 0 ]; then      \
            echo fail $$rc;         \
            exit 1;                 \
        fi;                         \
    done


distclean: uninstall clean 
	for dir in ${NODE_DIRS};            \
        do ${MAKE} -C $$dir distclean;  \
        rc=$$?;                         \
        if [ $$rc != 0 ]; then          \
            echo fail $$rc;             \
            exit 1;                     \
        fi;                             \
    done
	- rmdir ${INSTALL_DIR} 


install:
	for dir in ${NODE_DIRS};            \
        do ${MAKE} -C $$dir install;    \
        rc=$$?;                         \
        if [ $$rc != 0 ]; then          \
            echo fail $$rc;             \
            exit 1;                     \
        fi;                             \
    done


uninstall:
	for dir in ${NODE_DIRS};            \
        do ${MAKE} -C $$dir uninstall;  \
        rc=$$?;                         \
        if [ $$rc != 0 ]; then          \
            echo fail $$rc;             \
            exit 1;                     \
        fi;                             \
    done
	- rmdir ${INSTALL_DIR} 



inc_version:
	for dir in ${NODE_DIRS};            \
        do ${MAKE} -C $$dir inc_version;\
        rc=$$?;                         \
        if [ $$rc != 0 ]; then          \
            echo fail $$rc;             \
            exit 1;                     \
        fi;                             \
    done


svn_commit_version:
	for dir in ${NODE_DIRS};            \
        do ${MAKE} -C $$dir svn_commit_version;\
        rc=$$?;                         \
        if [ $$rc != 0 ]; then          \
            echo fail $$rc;             \
            exit 1;                     \
        fi;                             \
    done


# vim: set syntax=make:

