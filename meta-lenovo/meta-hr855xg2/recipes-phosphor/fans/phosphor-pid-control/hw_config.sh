# "Copyright (c) 2019-present Lenovo
# Licensed under BSD-3, see COPYING.BSD file for details. "

#!bin/bash

full_config_file="/usr/share/swampd/config.json_full"
full_config_file_ori="/usr/share/swampd/config.json_full_ori"
config_file="/usr/share/swampd/config.json"

if [[ -e $full_config_file ]]; then
    cp "$full_config_file" "$full_config_file_ori"
else
    echo -n "PID control file, config.json_full, does not exist, stop hw_config.sh"
    exit 1; 
fi

# Modify config.json based on the current HW

dimm_list=($(busctl tree --no-page xyz.openbmc_project.CPUSensor | egrep 'DIMM*' | sed -r -e 's/^.*temperature\///g'))

# Update empty read path for the present DIMMs
for ((dimm_idx=0; dimm_idx<${#dimm_list[@]}; dimm_idx++ )) ; do 
    # Write DIMM sensor names to "sensors" field.
    sed -r -i '/'${dimm_list[$dimm_idx]}'/,/readPath/s/""/"\/xyz\/openbmc_project\/sensors\/temperature\/'${dimm_list[$dimm_idx]}'"/' $full_config_file

    # Write DIMM sensor names to "zones" field.
    case ${dimm_list[$dimm_idx]} in 
        DIMM_[D-F][1-2]_CPU1) sed -r -i '/DIMM_1_ZONE/,/\"inputs\"/s/""/"'${dimm_list[$dimm_idx]}'"/1' $full_config_file;;
        DIMM_[A-C][1-2]_CPU1) sed -r -i '/DIMM_2_ZONE/,/\"inputs\"/s/""/"'${dimm_list[$dimm_idx]}'"/1' $full_config_file;;
        DIMM_[A-C][1-2]_CPU2) sed -r -i '/DIMM_3_ZONE/,/\"inputs\"/s/""/"'${dimm_list[$dimm_idx]}'"/1' $full_config_file;;
        DIMM_[D-F][1-2]_CPU2) sed -r -i '/DIMM_4_ZONE/,/\"inputs\"/s/""/"'${dimm_list[$dimm_idx]}'"/1' $full_config_file;;
        DIMM_[D-F][1-2]_CPU3) sed -r -i '/DIMM_5_ZONE/,/\"inputs\"/s/""/"'${dimm_list[$dimm_idx]}'"/1' $full_config_file;;
        DIMM_[A-C][1-2]_CPU3) sed -r -i '/DIMM_6_ZONE/,/\"inputs\"/s/""/"'${dimm_list[$dimm_idx]}'"/1' $full_config_file;;
        DIMM_[A-C][1-2]_CPU4) sed -r -i '/DIMM_7_ZONE/,/\"inputs\"/s/""/"'${dimm_list[$dimm_idx]}'"/1' $full_config_file;;
        DIMM_[D-F][1-2]_CPU4) sed -r -i '/DIMM_8_ZONE/,/\"inputs\"/s/""/"'${dimm_list[$dimm_idx]}'"/1' $full_config_file;;
    esac
done

# Check for any DIMM ZONEs which consist of empty inputs, and delete the ZONE.
rm_empty_zone_line=($(egrep -n '("",){5}' $full_config_file | cut -f1 -d:))

# DIMM_ZONE pid format should be under the following format for PID service. 
# {   
# "name": "DIMM_2_ZONE",
# "type": "temp",
# "inputs": ["","","","","","" ], --> "inputs" needs to be the 3rd row of DIMM_ZONE field. 
# ...
if [[ ${#rm_empty_zone_line[@]} -gt 0 ]] ; then
    for ((i=${#rm_empty_zone_line[@]}-1; i>=0; i--)); do
        rm_line=$((${rm_empty_zone_line[$i]}-3))
        sed -r -i ''$rm_line',/},/d' $full_config_file
    done
fi
unset rm_empty_zone_line

# Take care of the empty ,"" in each DIMM ZONE if avaliable.
dimm_zone_field=($(egrep -r 'DIMM*.*ZONE' $full_config_file | cut -f2 -d:))

for ((j=0; j<${#dimm_zone_field[@]}; j++)); do
    sed -r -i '/'${dimm_zone_field[$j]}'/,/\"inputs\"/s/(,"")*//g' $full_config_file
done
unset dimm_zone_field


# Once everything is completed, copy config.json_full to config.json and restore config.json_full to the original.
cp "$full_config_file" "$config_file"
cp "$full_config_file_ori" "$full_config_file"
rm "$full_config_file_ori"
