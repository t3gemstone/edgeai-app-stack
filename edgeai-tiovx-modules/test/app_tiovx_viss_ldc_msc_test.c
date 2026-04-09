/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
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

#include <tiovx_utils.h>
#include "tiovx_sensor_module.h"
#include "tiovx_viss_module.h"
#include "tiovx_multi_scaler_module.h"
#include "tiovx_ldc_module.h"

#define APP_BUFQ_DEPTH   (1)
#define NUM_ITERATIONS   (1)

#define APP_NUM_CH       (1)
#define APP_NUM_OUTPUTS  (1)

#define VISS_INPUT_WIDTH  1936
#define VISS_INPUT_HEIGHT 1096

#define VISS_OUTPUT_WIDTH  (VISS_INPUT_WIDTH)
#define VISS_OUTPUT_HEIGHT (VISS_INPUT_HEIGHT)

#define LDC_INPUT_WIDTH  VISS_OUTPUT_WIDTH
#define LDC_INPUT_HEIGHT VISS_OUTPUT_HEIGHT

#define LDC_OUTPUT_WIDTH  1920
#define LDC_OUTPUT_HEIGHT 1080

#define MSC_INPUT_WIDTH  LDC_OUTPUT_WIDTH
#define MSC_INPUT_HEIGHT LDC_OUTPUT_HEIGHT

#define MSC_OUTPUT_WIDTH  MSC_INPUT_WIDTH/2
#define MSC_OUTPUT_HEIGHT MSC_INPUT_HEIGHT/2

#define LDC_TABLE_WIDTH     (1920)
#define LDC_TABLE_HEIGHT    (1080)
#define LDC_DS_FACTOR       (2)
#define LDC_BLOCK_WIDTH     (64)
#define LDC_BLOCK_HEIGHT    (32)
#define LDC_PIXEL_PAD       (1)

typedef struct {

    /* OpenVX references */
    vx_context context;

    vx_graph   graph;

    SensorObj  sensorObj;

    TIOVXVISSModuleObj  vissObj;

    TIOVXMultiScalerModuleObj  scalerObj;

    TIOVXLDCModuleObj  ldcObj;

} AppObj;

static AppObj gAppObj;

static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_delete_graph(AppObj *obj);

vx_status app_modules_viss_msc_ldc_test(vx_int32 argc, vx_char* argv[])
{
    AppObj *obj = &gAppObj;
    vx_status status = VX_SUCCESS;

    status = app_init(obj);
    APP_PRINTF("App Init Done! \n");

    if(status == VX_SUCCESS)
    {
        status = app_create_graph(obj);
        APP_PRINTF("App Create Graph Done! \n");
    }

    if(status == VX_SUCCESS)
    {
        status = app_verify_graph(obj);
        APP_PRINTF("App Verify Graph Done! \n");
    }
    if (status == VX_SUCCESS)
    {
        status = app_run_graph(obj);
        APP_PRINTF("App Run Graph Done! \n");
    }

    app_delete_graph(obj);
    APP_PRINTF("App Delete Graph Done! \n");

    app_deinit(obj);
    APP_PRINTF("App De-init Done! \n");

    return status;
}

static vx_status app_init(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    /* Create OpenVx Context */
    obj->context = vxCreateContext();
    status = vxGetStatus((vx_reference) obj->context);

    if(status == VX_SUCCESS)
    {
        tivxHwaLoadKernels(obj->context);
    }

    /* VISS Init */
    if(status == VX_SUCCESS)
    {
        TIOVXVISSModuleObj *vissObj = &obj->vissObj;

        SensorObj *sensorObj = &obj->sensorObj;
        tiovx_querry_sensor(sensorObj);
        tiovx_init_sensor(sensorObj,"SENSOR_SONY_IMX390_UB953_D3");

        tivx_vpac_viss_params_init(&vissObj->params);

        snprintf(vissObj->dcc_config_file_path, TIVX_FILEIO_FILE_PATH_LENGTH, "%s", "/opt/imaging/imx390/linear/dcc_viss.bin");

        vissObj->input.bufq_depth = APP_BUFQ_DEPTH;

        vissObj->input.params.width  = VISS_INPUT_WIDTH;
        vissObj->input.params.height = VISS_INPUT_HEIGHT;
        vissObj->input.params.num_exposures = 1;
        vissObj->input.params.line_interleaved = vx_false_e;
        vissObj->input.params.meta_height_before = 0;
        vissObj->input.params.meta_height_after = 0;
        vissObj->input.params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        vissObj->input.params.format[0].msb = 11;

        vissObj->ae_awb_result_bufq_depth = APP_BUFQ_DEPTH;

        /* Enable NV12 output from VISS which can be tapped from output mux 2*/
        vissObj->output_select[0] = TIOVX_VISS_MODULE_OUTPUT_NA;
        vissObj->output_select[1] = TIOVX_VISS_MODULE_OUTPUT_NA;
        vissObj->output_select[2] = TIOVX_VISS_MODULE_OUTPUT_EN;
        vissObj->output_select[3] = TIOVX_VISS_MODULE_OUTPUT_NA;
        vissObj->output_select[4] = TIOVX_VISS_MODULE_OUTPUT_NA;

        /* As we are selecting output2, specify output2 image properties */
        vissObj->output2.bufq_depth   = APP_BUFQ_DEPTH;
        vissObj->output2.color_format = VX_DF_IMAGE_NV12;
        vissObj->output2.width        = VISS_OUTPUT_WIDTH;
        vissObj->output2.height       = VISS_OUTPUT_HEIGHT;

        vissObj->h3a_stats_bufq_depth = APP_BUFQ_DEPTH;

        /* Initialize modules */
        status = tiovx_viss_module_init(obj->context, vissObj, sensorObj);
        APP_PRINTF("VISS Init Done! \n");
    }

    /* LDC Init */
    if(status == VX_SUCCESS)
    {
        TIOVXLDCModuleObj *ldcObj = &obj->ldcObj;
        SensorObj *sensorObj = &obj->sensorObj;

        snprintf(ldcObj->dcc_config_file_path, TIVX_FILEIO_FILE_PATH_LENGTH, "%s", "/opt/imaging/imx390/wdr/dcc_ldc_wdr.bin");
        snprintf(ldcObj->lut_file_path, TIVX_FILEIO_FILE_PATH_LENGTH, "%s/raw_images/modules_test/imx390_ldc_lut_1920x1080.bin", EDGEAI_DATA_PATH);

        ldcObj->ldc_mode = TIOVX_MODULE_LDC_OP_MODE_DCC_DATA; //TIOVX_MODULE_LDC_OP_MODE_MESH_IMAGE
        ldcObj->en_out_image_write = 0;
        ldcObj->en_output1 = 0;

        ldcObj->input.bufq_depth = APP_BUFQ_DEPTH;
        ldcObj->input.color_format = VX_DF_IMAGE_NV12;
        ldcObj->input.width = LDC_INPUT_WIDTH;
        ldcObj->input.height = LDC_INPUT_HEIGHT;

        ldcObj->output0.bufq_depth = APP_BUFQ_DEPTH;
        ldcObj->output0.color_format = VX_DF_IMAGE_NV12;
        ldcObj->output0.width = LDC_OUTPUT_WIDTH;
        ldcObj->output0.height = LDC_OUTPUT_HEIGHT;

        ldcObj->init_x = 0;
        ldcObj->init_y = 0;
        ldcObj->table_width = LDC_TABLE_WIDTH;
        ldcObj->table_height = LDC_TABLE_HEIGHT;
        ldcObj->ds_factor = LDC_DS_FACTOR;
        ldcObj->out_block_width = LDC_BLOCK_WIDTH;
        ldcObj->out_block_height = LDC_BLOCK_HEIGHT;
        ldcObj->pixel_pad = LDC_PIXEL_PAD;

        /* Initialize modules */
        status = tiovx_ldc_module_init(obj->context, ldcObj, sensorObj);
        APP_PRINTF("LDC Init Done! \n");
    }

    /* MSC Init */
    if(status == VX_SUCCESS)
    {
        TIOVXMultiScalerModuleObj *scalerObj = &obj->scalerObj;
        vx_int32 out;

        scalerObj->num_channels = APP_NUM_CH;
        scalerObj->num_outputs = APP_NUM_OUTPUTS;
        scalerObj->input.bufq_depth = APP_BUFQ_DEPTH;
        for(out = 0; out < APP_NUM_OUTPUTS; out++)
        {
            scalerObj->output[out].bufq_depth = APP_BUFQ_DEPTH;
        }
        scalerObj->interpolation_method = VX_INTERPOLATION_BILINEAR;
        scalerObj->color_format = VX_DF_IMAGE_NV12;

        scalerObj->input.width = MSC_INPUT_WIDTH;
        scalerObj->input.height = MSC_INPUT_HEIGHT;

        for(out = 0; out < APP_NUM_OUTPUTS; out++)
        {
            scalerObj->output[out].width = MSC_OUTPUT_WIDTH;
            scalerObj->output[out].height = MSC_OUTPUT_HEIGHT;
        }

        tiovx_multi_scaler_module_crop_params_init(scalerObj);

        /* Initialize modules */
        status = tiovx_multi_scaler_module_init(obj->context, scalerObj);
        APP_PRINTF("Scaler Init Done! \n");
    }

    return status;
}

static void app_deinit(AppObj *obj)
{
    tiovx_deinit_sensor(&obj->sensorObj);

    tiovx_ldc_module_deinit(&obj->ldcObj);
    tiovx_viss_module_deinit(&obj->vissObj);
    tiovx_multi_scaler_module_deinit(&obj->scalerObj);

    tivxHwaUnLoadKernels(obj->context);

    vxReleaseContext(&obj->context);
}

static void app_delete_graph(AppObj *obj)
{
    tiovx_ldc_module_delete(&obj->ldcObj);
    tiovx_viss_module_delete(&obj->vissObj);
    tiovx_multi_scaler_module_delete(&obj->scalerObj);

    vxReleaseGraph(&obj->graph);
}

static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[8];
    vx_int32 graph_parameter_index;

    obj->graph = vxCreateGraph(obj->context);
    status = vxGetStatus((vx_reference)obj->graph);

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_viss_module_create(obj->graph, &obj->vissObj, NULL, NULL, TIVX_TARGET_VPAC_VISS1);
        status = tiovx_ldc_module_create(obj->graph, &obj->ldcObj, obj->vissObj.output2.arr[0], TIVX_TARGET_VPAC_LDC1);
        status = tiovx_multi_scaler_module_create(obj->graph, &obj->scalerObj, obj->ldcObj.output0.arr[0], TIVX_TARGET_VPAC_MSC1);
    }

    graph_parameter_index = 0;

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->vissObj.node, 1);

        obj->vissObj.ae_awb_result_graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->vissObj.ae_awb_result_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->vissObj.node, 3);

        obj->vissObj.input.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->vissObj.input.image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->scalerObj.node, 1);
        obj->scalerObj.output[0].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->scalerObj.output[0].image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = vxSetGraphScheduleConfig(obj->graph,
                    VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL,
                    graph_parameter_index,
                    graph_parameters_queue_params_list);
    }

    return status;
}

static vx_status app_verify_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    status = vxVerifyGraph(obj->graph);

    APP_PRINTF("App Verify Graph Done!\n");

    return status;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    char input_filename[100];
    char output_filename[100];

    sprintf(input_filename, "%s/raw_images/modules_test/imx390_raw_image_1936x1096_16bpp_exp0.raw", EDGEAI_DATA_PATH);
    sprintf(output_filename, "%s/output/imx390_960x540_capture_nv12.yuv", EDGEAI_DATA_PATH);

    tivx_raw_image input_o;
    vx_user_data_object aewb_o;
    vx_image output_o;

    TIOVXVISSModuleObj *vissObj = &obj->vissObj;
    TIOVXMultiScalerModuleObj *scalerObj = &obj->scalerObj;

    uint32_t num_refs;
    int32_t frame_count;

    frame_count = 0;
    while (frame_count < NUM_ITERATIONS)
    {
        APP_PRINTF("Running frame %d\n", frame_count);
        readRawImage(input_filename, vissObj->input.image_handle[0]);

        APP_PRINTF("Enqueueing input raw buffers!\n");
        vxGraphParameterEnqueueReadyRef(obj->graph, vissObj->input.graph_parameter_index, (vx_reference*)&vissObj->input.image_handle[0], 1);

        APP_PRINTF("Enqueueing ae-awb buffers!\n");
        vxGraphParameterEnqueueReadyRef(obj->graph, vissObj->ae_awb_result_graph_parameter_index, (vx_reference*)&vissObj->ae_awb_result_handle[0], 1);

        APP_PRINTF("Enqueueing output image buffers!\n");
        vxGraphParameterEnqueueReadyRef(obj->graph, scalerObj->output[0].graph_parameter_index, (vx_reference*)&scalerObj->output[0].image_handle[0], 1);

        APP_PRINTF("Processing!\n");
        status = vxScheduleGraph(obj->graph);
        if((vx_status)VX_SUCCESS != status) {
            APP_ERROR("Schedule Graph failed: %d!\n", status);
        }
        status = vxWaitGraph(obj->graph);
        if((vx_status)VX_SUCCESS != status) {
            APP_ERROR("Wait Graph failed: %d!\n", status);
        }

        vxGraphParameterDequeueDoneRef(obj->graph, vissObj->input.graph_parameter_index, (vx_reference*)&input_o, 1, &num_refs);
        vxGraphParameterDequeueDoneRef(obj->graph, scalerObj->output[0].graph_parameter_index, (vx_reference*)&output_o, 1, &num_refs);
        vxGraphParameterDequeueDoneRef(obj->graph, vissObj->ae_awb_result_graph_parameter_index, (vx_reference*)&aewb_o, 1, &num_refs);

        writeImage(output_filename, scalerObj->output[0].image_handle[0]);

        frame_count++;
    }

    return status;
}
