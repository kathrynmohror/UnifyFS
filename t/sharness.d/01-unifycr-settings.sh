#
# Export variables to control UNIFYCR runtime settings.
#

#
# Source a script that is dynamically generated by 0001-setup.t.
#
. $UNIFYCR_TEST_RUN_SCRIPT


# Common settings
UNIFYCR_MOUNTPOINT=${UNIFYCR_MOUNT_POINT:-$(mktemp -d)}
export UNIFYCR_MOUNTPOINT

# Server settings
UNIFYCR_META_DB_PATH=${UNIFYCR_META_DB_PATH:-$(mktemp -d)}
UNIFYCR_META_DB_NAME=${UNIFYCR_META_DB_NAME:-unifycr_db}
UNIFYCR_META_SERVER_RATIO=${UNIFYCR_META_SERVER_RATIO:-1}
UNIFYCR_LOG_DIR=${UNIFYCR_LOG_DIRECTORY:-$UNIFYCR_META_DB_PATH}
UNIFYCR_LOG_FILE=${UNIFYCR_LOG_FILE:-unifycrd_debuglog}
UNIFYCR_RUNSTATE_DIR=${UNIFYCR_RUNSTATE_DIR:-$UNIFYCR_META_DB_PATH}
export UNIFYCR_LOG_DIR
export UNIFYCR_LOG_FILE
export UNIFYCR_META_DB_NAME
export UNIFYCR_META_DB_PATH
export UNIFYCR_META_SERVER_RATIO
export UNIFYCR_RUNSTATE_DIR

# Client settings
UNIFYCR_SPILLOVER_ENABLED=${UNIFYCR_SPILLOVER_ENABLED:-"Y"}
UNIFYCR_SPILLOVER_DATA_DIR=${UNIFYCR_SPILLOVER_DATA_DIR:-$UNIFYCR_META_DB_PATH}
UNIFYCR_SPILLOVER_META_DIR=${UNIFYCR_SPILLOVER_META_DIR:-$UNIFYCR_META_DB_PATH}
export UNIFYCR_SPILLOVER_DATA_DIR
export UNIFYCR_SPILLOVER_META_DIR
export UNIFYCR_SPILLOVER_ENABLED
