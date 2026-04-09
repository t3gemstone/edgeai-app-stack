/*
 *
 * Copyright (c) 2023 Texas Instruments Incorporated
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
#include <tiovx_dl_post_proc_module.h>

static vx_status tiovx_dl_post_proc_module_create_config(vx_context context, TIOVXDLPostProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    tivxDLPostProcParams *params;
    vx_map_id map_id;

    obj->config = vxCreateUserDataObject(context, "tivxDLPostProcParams", sizeof(tivxDLPostProcParams), NULL );
    status = vxGetStatus((vx_reference)obj->config);

    if (VX_SUCCESS == status)
    {
        vxSetReferenceName((vx_reference)obj->config, "dl_post_proc_config");

        vxMapUserDataObject(obj->config, 0, sizeof(tivxDLPostProcParams), &map_id,
                        (void **)&params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        memcpy(params, &obj->params, sizeof(tivxDLPostProcParams));

        vxUnmapUserDataObject(obj->config, map_id);
    }

    return status;
}

static vx_status tiovx_dl_post_proc_module_create_input(vx_context context, TIOVXDLPostProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_image in_img;
    vx_int32 buf;

    vx_size tensor_sizes[TIOVX_MODULES_MAX_TENSOR_DIMS];
    vx_tensor in_tensor[TIOVX_MODULES_MAX_TENSORS];
    vx_uint32 in;
    // vx_int32 dim;

    /* Creating image input */
    if(obj->input_image.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Input image buffer queue depth %d greater than max supported %d!\n", obj->input_image.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->input_image.arr[buf]  = NULL;
        obj->input_image.image_handle[buf]  = NULL;
    }

    in_img  = vxCreateImage(context, obj->input_image.width, obj->input_image.height, obj->input_image.color_format);
    status = vxGetStatus((vx_reference)in_img);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->input_image.bufq_depth; buf++)
        {
            obj->input_image.arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_img, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->input_image.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Unable to create input array! \n");
                break;
            }
            obj->input_image.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input_image.arr[buf], 0);
        }

        vxReleaseImage(&in_img);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Unable to create input image template! \n");
    }

    /* Creating tensor input */
    for(in=0; in < obj->num_input_tensors ; in++)
    {
        if(obj->input_tensor[in].bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
        {
            TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Input tensor buffer queue depth %d greater than max supported %d!\n", obj->input_tensor[in].bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
            return VX_FAILURE;
        }
    }

    for(in=0; in < obj->num_input_tensors ; in++)
    {
        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->input_tensor[in].arr[buf]  = NULL;
            obj->input_tensor[in].tensor_handle[buf]  = NULL;
        }
    }

    for(in=0; in < obj->num_input_tensors ; in++)
    {
        tensor_sizes[0] = obj->input_tensor[in].dim_sizes[0];
        tensor_sizes[1] = obj->input_tensor[in].dim_sizes[1];
        tensor_sizes[2] = obj->input_tensor[in].dim_sizes[2];

        in_tensor[in]  = vxCreateTensor(context, obj->input_tensor[in].num_dims, tensor_sizes, obj->input_tensor[in].datatype, 0);
        status = vxGetStatus((vx_reference)in_tensor[in]);

        if(status == VX_SUCCESS)
        {
            for(buf = 0; buf < obj->input_tensor[in].bufq_depth; buf++)
            {
                obj->input_tensor[in].arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_tensor[in], obj->num_channels);
                status = vxGetStatus((vx_reference)obj->input_tensor[in].arr[buf]);
                
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Unable to create input array! \n");
                }

                obj->input_tensor[in].tensor_handle[buf] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)obj->input_tensor[in].arr[buf], 0);
            }
            vxReleaseTensor(&in_tensor[in]);
        }
        else
        {
            TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Unable to create input tensor template! \n");
        }
    }

    return status;
}

static vx_status tiovx_dl_post_proc_module_create_output(vx_context context, TIOVXDLPostProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_image out_img;
    vx_int32 buf;

    /* Creating image output */
    if(obj->output_image.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Output image buffer queue depth %d greater than max supported %d!\n", obj->output_image.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->output_image.arr[buf]  = NULL;
        obj->output_image.image_handle[buf]  = NULL;
    }

    out_img  = vxCreateImage(context, obj->output_image.width, obj->output_image.height, obj->output_image.color_format);
    status = vxGetStatus((vx_reference)out_img);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->output_image.bufq_depth; buf++)
        {
            obj->output_image.arr[buf]  = vxCreateObjectArray(context, (vx_reference)out_img, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->output_image.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Unable to create output array! \n");
                break;
            }
            obj->output_image.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output_image.arr[buf], 0);
        }

        vxReleaseImage(&out_img);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Unable to create output image template! \n");
    }

    if(obj->en_out_image_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];
        char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

        strcpy(file_path, obj->output_file_path);
        obj->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)obj->file_path);
        if(status == VX_SUCCESS)
        {
            vxAddArrayItems(obj->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Unable to create file path object for fileio!\n");
        }

        sprintf(file_prefix, "dl_post_proc_output");
        obj->file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)obj->file_prefix);
        if(status == VX_SUCCESS)
        {
            vxAddArrayItems(obj->file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Unable to create file prefix object for output!\n");
        }

        obj->write_cmd = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
        status = vxGetStatus((vx_reference)obj->write_cmd);
        if(status == VX_SUCCESS)
        {
            TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Unable to create write cmd object for output!\n");
        }
    }
    else
    {
        obj->file_path   = NULL;
        obj->file_prefix = NULL;
        obj->write_node  = NULL;
        obj->write_cmd   = NULL;
    }

    return status;
}

vx_status tiovx_dl_post_proc_module_init(vx_context context, TIOVXDLPostProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    obj->kernel = NULL;

    status = tiovx_dl_post_proc_module_create_config(context, obj);

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dl_post_proc_module_create_input(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dl_post_proc_module_create_output(context, obj);
    }

    if(status == VX_SUCCESS)
    {
        obj->kernel = tivxAddKernelDLPostProc(context, obj->num_input_tensors);
        status = vxGetStatus((vx_reference)obj->kernel);
    }

    return status;
}

vx_status tiovx_dl_post_proc_module_deinit(TIOVXDLPostProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_int32 buf;
    vx_uint32 in;

    TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Releasing config handle!\n");
    status = vxReleaseUserDataObject(&obj->config);

    for(buf = 0; buf < obj->input_image.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Releasing input image handle!\n");
            status = vxReleaseImage(&obj->input_image.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Releasing input image arr!\n");
            status = vxReleaseObjectArray(&obj->input_image.arr[buf]);
        }
    }

    for(in = 0; in < obj->num_input_tensors; in++)
    {
        for(buf = 0; buf < obj->input_tensor[in].bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Releasing input tensor handle!\n");
                status = vxReleaseTensor(&obj->input_tensor[in].tensor_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Releasing input tensor arr!\n");
                status = vxReleaseObjectArray(&obj->input_tensor[in].arr[buf]);
            }
        }
    }

    for(buf = 0; buf < obj->output_image.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Releasing output image handle!\n");
            status = vxReleaseImage(&obj->output_image.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Releasing input image arr!\n");
            status = vxReleaseObjectArray(&obj->output_image.arr[buf]);
        }
    }

    if(obj->en_out_image_write == 1)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Releasing output path array!\n");
            status = vxReleaseArray(&obj->file_path);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Releasing output file prefix array!\n");
            status = vxReleaseArray(&obj->file_prefix);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Releasing output write command object!\n");
            status = vxReleaseUserDataObject(&obj->write_cmd);
        }
    }

    return status;
}

vx_status tiovx_dl_post_proc_module_delete(TIOVXDLPostProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    if(obj->node != NULL)
    {
        TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Releasing node reference!\n");
        status = vxReleaseNode(&obj->node);
    }
    if(obj->write_node != NULL)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Releasing write node reference!\n");
            status = vxReleaseNode(&obj->write_node);
        }
    }
    if(obj->kernel != NULL)
    {
        if(status == VX_SUCCESS)
            vxRemoveKernel(obj->kernel);
    }

    return status;
}

vx_status tiovx_dl_post_proc_module_create(vx_graph graph, TIOVXDLPostProcModuleObj *obj, vx_object_array input_image_arr, TensorObj input_tensor_arr[], const char* target_string)
{
    vx_status status = VX_SUCCESS;

    vx_image input_image;
    vx_tensor input_tensor[TIOVX_MODULES_MAX_TENSORS];
    vx_image output_image;

    vx_uint32 in;

    if(input_image_arr != NULL)
    {
        input_image = (vx_image)vxGetObjectArrayItem((vx_object_array)input_image_arr, 0);
    }
    else
    {
        if(obj->input_image.arr[0] != NULL)
        {
            input_image = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input_image.arr[0], 0);
        }
        else
        {
            input_image = NULL;
        }
    }

    if(input_tensor_arr != NULL)
    {
        for(in = 0; in < obj->num_input_tensors; in++)
        {
            input_tensor[in] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)input_tensor_arr[in].arr[0], 0);
        }
    }
    else
    {
        for(in = 0; in < obj->num_input_tensors; in++)
        {
            if(obj->input_tensor[in].arr[0] != NULL)
            {
                input_tensor[in] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)obj->input_tensor[in].arr[0], 0);
            }
            else
            {
                input_tensor[in] = NULL;
            }
        }
    }

    if(obj->output_image.arr[0] != NULL)
    {
        output_image = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output_image.arr[0], 0);
    }
    else
    {
        output_image = NULL;
    }

    /* Modify post proc kernel to take care of array of input tensors */
    obj->node = tivxDLPostProcNode(graph, obj->kernel, obj->config, input_image, input_tensor, output_image);
    status = vxGetStatus((vx_reference)obj->node);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->node, VX_TARGET_STRING, target_string);

        vx_bool replicate[8];
        replicate[TIVX_DL_POST_PROC_CONFIG_IDX] = vx_false_e;
        replicate[TIVX_DL_POST_PROC_INPUT_IMAGE_IDX] = vx_true_e;
        replicate[TIVX_DL_POST_PROC_OUTPUT_IMAGE_IDX] = vx_true_e;

        for(in = 0; in < obj->num_input_tensors; in++)
        {
            replicate[TIVX_DL_POST_PROC_INPUT_TENSOR_START_IDX + in] = vx_true_e;
        }

        vxReplicateNode(graph, obj->node, replicate, TIVX_DL_POST_PROC_BASE_PARAMS + obj->num_input_tensors);

        if(obj->en_out_image_write == 1)
        {
            if(output_image != NULL)
            {
                status = tiovx_dl_post_proc_module_add_write_output_node(graph, obj);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Unable to create write node for output!\n");
                }
            }
        }
    }
    else
    {
        TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Unable to create node! \n");
    }

    if(input_image != NULL)
        vxReleaseImage(&input_image);

    for(in = 0; in < obj->num_input_tensors; in++)
    {    
        if(input_tensor[in] != NULL)
        vxReleaseTensor(&input_tensor[in]);
    }

    if(output_image != NULL)
        vxReleaseImage(&output_image);

    return status;
}

vx_status tiovx_dl_post_proc_module_release_buffers(TIOVXDLPostProcModuleObj *obj)
{
    vx_status   status = VX_SUCCESS;

    void        *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries, in;
    vx_int32    bufq, ch;

    /* Free input image handles */
    for(bufq = 0; bufq < obj->input_image.bufq_depth; bufq++)
    {
        for(ch = 0; ch < obj->num_channels; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->input_image.arr[bufq], ch);
            status = vxGetStatus((vx_reference)ref);

            if((vx_status)VX_SUCCESS == status)
            {
                /* Export handles to get valid size information. */
                status = tivxReferenceExportHandle(ref,
                                                   virtAddr,
                                                   size,
                                                   TIOVX_MODULES_MAX_REF_HANDLES,
                                                   &numEntries);

                if((vx_status)VX_SUCCESS == status)
                {
                    vx_int32 ctr;
                    /* Currently the vx_image buffers are alloated in one shot for multiple planes.
                        So if we are freeing this buffer then we need to get only the first plane
                        pointer address but add up the all the sizes to free the entire buffer */
                    vx_uint32 freeSize = 0;
                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        freeSize += size[ctr];
                    }

                    if(virtAddr[0] != NULL)
                    {
                        TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
                        tivxMemFree(virtAddr[0], freeSize, TIVX_MEM_EXTERNAL);
                    }

                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        virtAddr[ctr] = NULL;
                    }

                    /* Assign NULL handles to the OpenVx objects as it will avoid
                        doing a tivxMemFree twice, once now and once during release */
                    status = tivxReferenceImportHandle(ref,
                                                    (const void **)virtAddr,
                                                    (const uint32_t *)size,
                                                    numEntries);
                }
                vxReleaseReference(&ref);
            }
        }
    }

    /* Free input tensor handles */
    for(in = 0; in < obj->num_input_tensors; in++)
    {
        for(bufq = 0; bufq < obj->input_tensor[in].bufq_depth; bufq++)
        {
            for(ch = 0; ch < obj->num_channels; ch++)
            {
                vx_reference ref = vxGetObjectArrayItem(obj->input_tensor[in].arr[bufq], ch);
                status = vxGetStatus((vx_reference)ref);

                if((vx_status)VX_SUCCESS == status)
                {
                    /* Export handles to get valid size information. */
                    status = tivxReferenceExportHandle(ref,
                                                        virtAddr,
                                                        size,
                                                        TIOVX_MODULES_MAX_REF_HANDLES,
                                                        &numEntries);

                    if((vx_status)VX_SUCCESS == status)
                    {
                        vx_int32 ctr;
                        /* Currently the vx_image buffers are alloated in one shot for multiple planes.
                            So if we are freeing this buffer then we need to get only the first plane
                            pointer address but add up the all the sizes to free the entire buffer */
                        vx_uint32 freeSize = 0;
                        for(ctr = 0; ctr < numEntries; ctr++)
                        {
                            freeSize += size[ctr];
                        }

                        if(virtAddr[0] != NULL)
                        {
                            TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Freeing output, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
                            tivxMemFree(virtAddr[0], freeSize, TIVX_MEM_EXTERNAL);
                        }

                        for(ctr = 0; ctr < numEntries; ctr++)
                        {
                            virtAddr[ctr] = NULL;
                        }

                        /* Assign NULL handles to the OpenVx objects as it will avoid
                            doing a tivxMemFree twice, once now and once during release */
                        status = tivxReferenceImportHandle(ref,
                                                        (const void **)virtAddr,
                                                        (const uint32_t *)size,
                                                        numEntries);
                    }
                    vxReleaseReference(&ref);
                }
            }
        }
    }

    /* Free output handles */
    for(bufq = 0; bufq < obj->output_image.bufq_depth; bufq++)
    {
        for(ch = 0; ch < obj->num_channels; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->output_image.arr[bufq], ch);
            status = vxGetStatus((vx_reference)ref);

            if((vx_status)VX_SUCCESS == status)
            {
                /* Export handles to get valid size information. */
                status = tivxReferenceExportHandle(ref,
                                                    virtAddr,
                                                    size,
                                                    TIOVX_MODULES_MAX_REF_HANDLES,
                                                    &numEntries);

                if((vx_status)VX_SUCCESS == status)
                {
                    vx_int32 ctr;
                    /* Currently the vx_image buffers are alloated in one shot for multiple planes.
                        So if we are freeing this buffer then we need to get only the first plane
                        pointer address but add up the all the sizes to free the entire buffer */
                    vx_uint32 freeSize = 0;
                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        freeSize += size[ctr];
                    }

                    if(virtAddr[0] != NULL)
                    {
                        TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] Freeing output, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
                        tivxMemFree(virtAddr[0], freeSize, TIVX_MEM_EXTERNAL);
                    }

                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        virtAddr[ctr] = NULL;
                    }

                    /* Assign NULL handles to the OpenVx objects as it will avoid
                        doing a tivxMemFree twice, once now and once during release */
                    status = tivxReferenceImportHandle(ref,
                                                    (const void **)virtAddr,
                                                    (const uint32_t *)size,
                                                    numEntries);
                }
                vxReleaseReference(&ref);
            }
        }
    }

    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
    }

    return status;
}

vx_status tiovx_dl_post_proc_module_add_write_output_node(vx_graph graph, TIOVXDLPostProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    /* Need to improve this section, currently one write node can take only one image. */
    vx_image output_image = (vx_image)vxGetObjectArrayItem(obj->output_image.arr[0], 0);
    obj->write_node = tivxWriteImageNode(graph, output_image, obj->file_path, obj->file_prefix);
    vxReleaseImage(&output_image);

    status = vxGetStatus((vx_reference)obj->write_node);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->write_node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, obj->write_node, replicate, 3);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] Unable to create fileio write node for storing outputs! \n");
    }

    return (status);
}

vx_status tiovx_dl_post_proc_module_send_write_output_cmd(TIOVXDLPostProcModuleObj *obj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
{
    vx_status status = VX_SUCCESS;

    tivxFileIOWriteCmd write_cmd;

    write_cmd.start_frame = start_frame;
    write_cmd.num_frames = num_frames;
    write_cmd.num_skip = num_skip;

    status = vxCopyUserDataObject(obj->write_cmd, 0, sizeof(tivxFileIOWriteCmd),\
                &write_cmd, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

    if((vx_status)VX_SUCCESS == status)
    {
        vx_reference refs[2];

        refs[0] = (vx_reference)obj->write_cmd;

        status = tivxNodeSendCommand(obj->write_node, TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
                                TIVX_FILEIO_CMD_SET_FILE_WRITE,
                                refs, 1u);

        if(VX_SUCCESS != status)
        {
            TIOVX_MODULE_ERROR("[DL-POST-PROC-MODULE] write node send command failed!\n");
        }

        TIOVX_MODULE_PRINTF("[DL-POST-PROC-MODULE] write node send command success!\n");
    }

    return (status);
}
