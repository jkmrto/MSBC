#   L7 classification plugin Makefile
#
#HEADER

# Check SDK path

 
#--------------------------------------
include $(TIE_SDK)/mk/plugin_options.mk
#--------------------------------------

# Specify here additional options/variables needed by this plugin

# Specify here additional OBJECTS needed by this plugin (will be linked together with the plugin shared object) 
OBJECTS	+= Aux_Functions.o Kmeans.o NormalizeFunctions.o MSBC_train.o MSBC_conf.o


# Specify here additional compiler flags needed by this plugin
CFLAGS +=

# Specify here additional linker flags needed by this plugin
LDFLAGS +=

# Specify here static libraries needed by this plugin (will be linked together with the plugin shared object)
LIBS	:=

# Specify here files and folders to be copied to plugin destination folder togheter with the plugin.
# Each folder is recursively processed skipping hidden files/folders. 
# Each file will be copied in update mode (overwrite only if newer).
COPY	:= foreign_MSBC_signature_file.txt foreign_MSBC_map_file.txt TIE_apps.txt own_MSBC_map_file.txt own_MSBC_signature_file.txt MSBC_conf.txt
#------------------------------------
include $(TIE_SDK)/mk/plugin_rules.mk
#------------------------------------
