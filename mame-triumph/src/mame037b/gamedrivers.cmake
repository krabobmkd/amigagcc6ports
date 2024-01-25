# to be included
# gather the list of gamedrivers source to be included in project.
# also gather list of corresponding CPU sources.
# just change MGD_XXXX variables with cmake -D option to enable machines or not in the project. 
option(MGD_ALL ON)
option(MGD_SEGASYSTEM16 "" OFF )
option(MGD_NEOMAME "" OFF)


if(MGD_SEGASYSTEM16)
endif()