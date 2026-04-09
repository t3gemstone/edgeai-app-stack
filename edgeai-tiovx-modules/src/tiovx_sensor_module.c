/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <tiovx_sensor_module.h>

static char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];

vx_status tiovx_sensor_module_query(SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;
    char* sensor_list[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_uint16 selectedSensor = 0xFFF;
    vx_uint8 sensors_detected[ISS_SENSORS_MAX_SUPPORTED_SENSOR];
    vx_bool sensorSelected = vx_false_e;
    vx_bool ldcSelected = vx_false_e;
    int32_t i;

    memset(availableSensorNames, 0, ISS_SENSORS_MAX_SUPPORTED_SENSOR*ISS_SENSORS_MAX_NAME);
    for(i = 0; i < ISS_SENSORS_MAX_SUPPORTED_SENSOR; i++)
    {
        sensor_list[i] = availableSensorNames[i];
    }

    memset(&sensorObj->sensorParams, 0, sizeof(IssSensor_CreateParams));
    status = appEnumerateImageSensor(sensor_list, &sensorObj->num_sensors_found);
    if(VX_SUCCESS != status)
    {
        TIOVX_MODULE_ERROR("appCreateImageSensor failed, returned status = %d\n", status);
        return status;
    }

    if(sensorObj->is_interactive == 1)
    {
        vx_char ch = 0;
        int ch_id;

        while(sensorSelected != vx_true_e)
        {
            printf("%d sensor(s) found \n", sensorObj->num_sensors_found);
            printf("Supported sensor list: \n");
            for(i = 0; i < sensorObj->num_sensors_found; i++)
            {
                printf("%c : %s \n", i+'a', sensor_list[i]);
            }

            printf("Select a sensor above or press '0' to autodetect the sensor \n");
            ch = getchar();
            if(ch == '0')
            {
                uint8_t num_sensors_found= 0;
                uint16_t channel_mask = 0xFFF;
                /*AutoDetect*/
                memset(sensors_detected, 0xFFF, ISS_SENSORS_MAX_SUPPORTED_SENSOR);
                status = appDetectImageSensor(sensors_detected, &num_sensors_found, channel_mask);
                if(0 == status)
                {
                    int detected_sensor_index;
                    char * detected_sensor_name;
                    for(ch_id=0;ch_id<ISS_SENSORS_MAX_SUPPORTED_SENSOR;ch_id++)
                    {
                        detected_sensor_index = sensors_detected[ch_id];
                        if(detected_sensor_index < ISS_SENSORS_MAX_SUPPORTED_SENSOR)
                        {
                            detected_sensor_name = sensor_list[detected_sensor_index];
                            printf("sensor detected at channel %d = %s \n", ch_id, detected_sensor_name);
                        }
                        else
                        {
                            printf("sensor detected at channel %d = None \n", ch_id);
                        }
                    }
                }
                else
                {
                    TIOVX_MODULE_ERROR("appDetectImageSensor failed with error = 0x%x \n", status);
                }
            }
            else
            {
                selectedSensor = ch - 'a';
                /*Assume all cameras are identical*/
                for(ch_id=0;ch_id<ISS_SENSORS_MAX_SUPPORTED_SENSOR;ch_id++)
                {
                    sensors_detected[ch_id] = selectedSensor;
                }
            }

            //TODO : sensor properties need to be queried. 
            //Currently this is supported for cam0 only
            selectedSensor = sensors_detected[0];
            if(selectedSensor > (sensorObj->num_sensors_found-1))
            {
                TIOVX_MODULE_ERROR("Invalid selection %d. Try again \n", selectedSensor);
            }
            else
            {
                snprintf(sensorObj->sensor_name, ISS_SENSORS_MAX_NAME, "%s", sensor_list[selectedSensor]);

                printf("Sensor selected : %s\n", sensorObj->sensor_name);

                printf("Querying %s \n", sensorObj->sensor_name);
                status = appQueryImageSensor(sensorObj->sensor_name, &sensorObj->sensorParams);
                if(VX_SUCCESS != status)
                {
                    TIOVX_MODULE_ERROR("appQueryImageSensor returned %d\n", status);
                    return status;
                }

                if(sensorObj->sensorParams.sensorInfo.raw_params.format[0].pixel_container == VX_DF_IMAGE_UYVY)
                {
                    /* sensor_out_format = 1 for YUV */
                    sensorObj->sensor_out_format = 1;
                }
                else
                {   
                    /* sensor_out_format = 1 for RAW */
                    sensorObj->sensor_out_format = 0;
                }

                sensorSelected = vx_true_e;
            }
        }

        while (ldcSelected != vx_true_e)
        {
            fflush (stdin);
            printf ("LDC Selection Yes(1)/No(0)\n");
            ch = getchar();
            sensorObj->enable_ldc = ch - '0';

            if((sensorObj->enable_ldc > 1) || (sensorObj->enable_ldc < 0))
            {
                TIOVX_MODULE_ERROR("Invalid selection %c. Try again \n", ch);
            }
            else
            {
                ldcSelected = vx_true_e;
            }
        }

        sensorObj->num_cameras_enabled = 0;
        while(sensorObj->num_cameras_enabled == 0)
        {
            char c;
            int ret;
            int num_cameras;
            fflush(stdin);
            printf("Max number of cameras supported by sensor %s = %d \n", sensorObj->sensor_name, sensorObj->sensorParams.num_channels);
            printf("Please enter number of cameras to be enabled \n");
            ret = scanf("%d", &num_cameras);
            while ((c = getchar()) != '\n' && c != EOF);
            sensorObj->num_cameras_enabled = num_cameras;

            if( (1==ret) &&
                ( (sensorObj->num_cameras_enabled > sensorObj->sensorParams.num_channels) ||
                (sensorObj->num_cameras_enabled <= 0) ) )
            {
                sensorObj->num_cameras_enabled = 0;
                TIOVX_MODULE_ERROR("Invalid selection %d. Try again \n", num_cameras);
            }
            else if((sensorObj->num_cameras_enabled > ISS_SENSORS_MAX_SUPPORTED_SENSOR))
            {
                sensorObj->num_cameras_enabled = 0;
                TIOVX_MODULE_ERROR("Number of cameras should not be greater than %d. \n", ISS_SENSORS_MAX_SUPPORTED_SENSOR);
                TIOVX_MODULE_ERROR("Invalid selection %d. Try again \n", num_cameras);
            }
        }
        sensorObj->ch_mask = (1<<sensorObj->num_cameras_enabled) - 1;
    }
    else /* (sensorObj->is_interactive != 1), that means non interactive case */
    {
        /* Need to populate sensorObj->sensor_index from application */
        selectedSensor = sensorObj->sensor_index;
        if(selectedSensor > (sensorObj->num_sensors_found-1))
        {
            TIOVX_MODULE_ERROR("Invalid selection from application: %d. Exiting. \n", selectedSensor);
            return -1;
        }
        else
        {
            snprintf(sensorObj->sensor_name, ISS_SENSORS_MAX_NAME, "%s", sensor_list[selectedSensor]);
            printf("Sensor selected : %s\n", sensorObj->sensor_name);

            printf("Querying %s \n", sensorObj->sensor_name);
            status = appQueryImageSensor(sensorObj->sensor_name, &sensorObj->sensorParams);
            if(VX_SUCCESS != status)
            {
                TIOVX_MODULE_ERROR("appQueryImageSensor returned %d\n", status);
                return status;
            }

            if(sensorObj->sensorParams.sensorInfo.raw_params.format[0].pixel_container == VX_DF_IMAGE_UYVY)
            {
                /* sensor_out_format = 1 for YUV */
                sensorObj->sensor_out_format = 1;
            }
            else
            {
                /* sensor_out_format = 0 for RAW */
                sensorObj->sensor_out_format = 0;
            }

            sensorSelected = vx_true_e;
        }

        if(sensorObj->ch_mask > 0)
        {
            vx_uint32 mask = sensorObj->ch_mask;
            sensorObj->num_cameras_enabled = 0;
            while(mask > 0)
            {
                if(mask & 0x1)
                {
                    sensorObj->num_cameras_enabled++;
                }            
                mask = mask >> 1;
            }
        }
    }

    /*
    Check for supported sensor features.
    It is upto the application to decide which features should be enabled.
    This demo app enables WDR, DCC and 2A if the sensor supports it.
    */

    sensorObj->sensor_features_supported = sensorObj->sensorParams.sensorInfo.features;

    if(ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE == (sensorObj->sensor_features_supported & ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE))
    {
        TIOVX_MODULE_PRINTF("WDR mode is supported \n");
        sensorObj->sensor_features_enabled |= ISS_SENSOR_FEATURE_COMB_COMP_WDR_MODE;
        sensorObj->sensor_wdr_enabled = 1;
    }else
    {
        TIOVX_MODULE_PRINTF("WDR mode is not supported. Defaulting to linear \n");
        sensorObj->sensor_features_enabled |= ISS_SENSOR_FEATURE_LINEAR_MODE;
        sensorObj->sensor_wdr_enabled = 0;
    }

    if(ISS_SENSOR_FEATURE_MANUAL_EXPOSURE == (sensorObj->sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_EXPOSURE))
    {
        TIOVX_MODULE_PRINTF("Expsoure control is supported \n");
        sensorObj->sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_EXPOSURE;
        sensorObj->sensor_exp_control_enabled = 1;
    }

    if(ISS_SENSOR_FEATURE_MANUAL_GAIN == (sensorObj->sensor_features_supported & ISS_SENSOR_FEATURE_MANUAL_GAIN))
    {
        TIOVX_MODULE_PRINTF("Gain control is supported \n");
        sensorObj->sensor_features_enabled |= ISS_SENSOR_FEATURE_MANUAL_GAIN;
        sensorObj->sensor_gain_control_enabled = 1;
    }

    if(ISS_SENSOR_FEATURE_CFG_UC1 == (sensorObj->sensor_features_supported & ISS_SENSOR_FEATURE_CFG_UC1))
    {
        if(sensorObj->usecase_option == TIOVX_SENSOR_MODULE_FEATURE_CFG_UC1)
        {
            TIOVX_MODULE_PRINTF("CMS Usecase is supported \n");
            sensorObj->sensor_features_enabled |= ISS_SENSOR_FEATURE_CFG_UC1;
        }
    }

    if(ISS_SENSOR_FEATURE_DCC_SUPPORTED == (sensorObj->sensor_features_supported & ISS_SENSOR_FEATURE_DCC_SUPPORTED))
    {
        sensorObj->sensor_features_enabled |= ISS_SENSOR_FEATURE_DCC_SUPPORTED;
        sensorObj->sensor_dcc_enabled = 1;
        TIOVX_MODULE_PRINTF("Sensor DCC is enabled \n");
    }else
    {
        sensorObj->sensor_dcc_enabled = 0;
        TIOVX_MODULE_PRINTF("Sensor DCC is disabled \n");
    }

    sensorObj->image_width   = sensorObj->sensorParams.sensorInfo.raw_params.width;
    sensorObj->image_height  = sensorObj->sensorParams.sensorInfo.raw_params.height;

    TIOVX_MODULE_PRINTF("Sensor width = %d\n", sensorObj->sensorParams.sensorInfo.raw_params.width);
    TIOVX_MODULE_PRINTF("Sensor height = %d\n", sensorObj->sensorParams.sensorInfo.raw_params.height);
    TIOVX_MODULE_PRINTF("Sensor DCC ID = %d\n", sensorObj->sensorParams.dccId);
    TIOVX_MODULE_PRINTF("Sensor Supported Features = 0x%08X\n", sensorObj->sensor_features_supported);
    TIOVX_MODULE_PRINTF("Sensor Enabled Features = 0x%08X\n", sensorObj->sensor_features_enabled);

    return status;
}

vx_status tiovx_sensor_module_init(SensorObj *sensorObj, char *objName)
{
    vx_status status = VX_SUCCESS;
    int32_t sensor_init_status = -1;
    int32_t ch_mask = sensorObj->ch_mask;

    sensor_init_status = appInitImageSensor(sensorObj->sensor_name, sensorObj->sensor_features_enabled, ch_mask);
    if(0 != sensor_init_status)
    {
        TIOVX_MODULE_ERROR("Error initializing sensor %s \n", sensorObj->sensor_name);
        status = VX_FAILURE;
    }

    return status;
}

void tiovx_sensor_module_deinit(SensorObj *sensorObj)
{
    appDeInitImageSensor(sensorObj->sensor_name);
}

void tiovx_sensor_module_params_init(SensorObj *sensorObj)
{
    strcpy(sensorObj->sensor_name, SENSOR_SONY_IMX390_UB953_D3);
    sensorObj->num_sensors_found = 0;
    sensorObj->sensor_features_enabled = 0;
    sensorObj->sensor_features_supported = 0;
    sensorObj->sensor_dcc_enabled = 0;
    sensorObj->sensor_wdr_enabled = 0;
    sensorObj->sensor_exp_control_enabled = 0;
    sensorObj->sensor_gain_control_enabled = 0;
    sensorObj->ch_mask = 1;
    sensorObj->enable_ldc = 1;
    sensorObj->num_cameras_enabled = 1;
    sensorObj->usecase_option = TIOVX_SENSOR_MODULE_FEATURE_CFG_UC0;
    sensorObj->is_interactive = 0;
    sensorObj->sensor_index = 0;
}

vx_status tiovx_sensor_module_start(SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    status = appStartImageSensor(sensorObj->sensor_name, sensorObj->ch_mask);

    return status;
}

vx_status tiovx_sensor_module_stop(SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    status = appStopImageSensor(sensorObj->sensor_name, sensorObj->ch_mask);

    return status;
}



/*****************************************************************************/

/* THE BELOW APIS ARE ONLY USED BY EDGEAI-GST-APP STACK (EDGEAI_GST_PLUGINS) */

vx_status tiovx_querry_sensor(SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    return (status);
}

vx_status tiovx_init_sensor(SensorObj *sensorObj, char *objName)
{
    vx_status status = VX_SUCCESS;
    sensorObj->sensor_dcc_enabled=1;
    sensorObj->sensor_exp_control_enabled=0;
    sensorObj->sensor_gain_control_enabled=0;
    sensorObj->sensor_wdr_enabled=0;
    sensorObj->num_cameras_enabled=1;
    sensorObj->ch_mask=1;
    snprintf(sensorObj->sensor_name, ISS_SENSORS_MAX_NAME, "%s", objName);

    TIOVX_MODULE_PRINTF("[SENSOR-MODULE] Sensor name = %s\n", sensorObj->sensor_name);

    if(strcmp(sensorObj->sensor_name, "SENSOR_SONY_IMX390_UB953_D3") == 0)
    {
        sensorObj->sensorParams.dccId=390;
    }
    else if(strcmp(sensorObj->sensor_name, "SENSOR_ONSEMI_AR0820_UB953_LI") == 0)
    {
        sensorObj->sensorParams.dccId=820;
    }
    else if(strcmp(sensorObj->sensor_name, "SENSOR_ONSEMI_AR0233_UB953_MARS") == 0)
    {
        sensorObj->sensorParams.dccId=233;
    }
    else if(strcmp(sensorObj->sensor_name, "SENSOR_SONY_IMX219_RPI") == 0)
    {
        sensorObj->sensorParams.dccId=219;
    }
    else if(strcmp(sensorObj->sensor_name, "SENSOR_OV2312_UB953_LI") == 0)
    {
        sensorObj->sensorParams.dccId=2312;
    }
    else if(strcmp(sensorObj->sensor_name, "SENSOR_OX05B1S") == 0)
    {
        sensorObj->sensorParams.dccId=5;
    }
    else
    {
        TIOVX_MODULE_ERROR("[SENSOR-MODULE] Invalid sensor name\n");
        status = VX_FAILURE;
    }

    TIOVX_MODULE_PRINTF("[SENSOR-MODULE] Dcc ID = %d\n", sensorObj->sensorParams.dccId);

    return status;
}

void tiovx_deinit_sensor(SensorObj *sensorObj)
{
    return;
}