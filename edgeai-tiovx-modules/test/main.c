/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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
#include <stdio.h>
#include <TI/tivx.h>
#include <app_init.h>
#include <stdlib.h>
#include <getopt.h>

char *EDGEAI_DATA_PATH;
char *CHOICES[] = {"multiscaler","colorconvert","dlcolorconvert","mosaic",
                   "dlpreproc", "ldc", "viss", "pyramid", "dof", "dof-viz",
                   "sde", "sde-viz", "viss-ldc-msc"};

int main(int argc, char *argv[])
{
    int status = 0;
    int i = 0;
    size_t total_choices = sizeof(CHOICES)/ sizeof(CHOICES[0]);

    if (argc > 1)
    {
        for(i = 0; i < total_choices; i++)
        {
            CHOICES[i] = NULL;
        }
    }

    for(i = 0; optind < argc && i < total_choices; optind++, i++)
    {
        CHOICES[i] = argv[optind];
    }

    EDGEAI_DATA_PATH = getenv("EDGEAI_DATA_PATH");
    if (EDGEAI_DATA_PATH == NULL)
    {
      APP_ERROR("EDGEAI_DATA_PATH Not Defined!!\n");
    }

    status = appInit();


    for(i = 0; i < total_choices && CHOICES[i] != NULL; i++)
    {

        if(status == 0 && strcmp(CHOICES[i], "multiscaler") == 0)
        {
            printf("[Running multi-scaler module test]\n");
            int app_modules_scaler_test(int argc, char* argv[]);

            status = app_modules_scaler_test(argc, argv);
        }

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
        else if(status == 0 && strcmp(CHOICES[i], "colorconvert") == 0)
        {
            printf("[Running color convert module test]\n");
            int app_modules_color_convert_test(int argc, char* argv[]);

            status = app_modules_color_convert_test(argc, argv);
        }
#endif

        else if(status == 0 && strcmp(CHOICES[i], "dlcolorconvert") == 0)
        {
            printf("[Running DL color convert module test]\n");
            int app_modules_dl_color_convert_test(int argc, char* argv[]);

            status = app_modules_dl_color_convert_test(argc, argv);
        }

        else if(status == 0 && strcmp(CHOICES[i], "mosaic") == 0)
        {
            printf("[Running image mosaic module test]\n");
            int app_modules_img_mosaic_test(int argc, char* argv[]);

            status = app_modules_img_mosaic_test(argc, argv);

        }

        else if(status == 0 && strcmp(CHOICES[i], "dlpreproc") == 0)
        {
            printf("[Running DL pre-proc module test]\n");
            int app_modules_dl_pre_proc_test(int argc, char* argv[]);

            status = app_modules_dl_pre_proc_test(argc, argv);

        }

        else if(status == 0 && strcmp(CHOICES[i], "ldc") == 0)
        {
            printf("[Running LDC module test]\n");
            int app_modules_ldc_test(int argc, char* argv[]);

            status = app_modules_ldc_test(argc, argv);
        }

        else if(status == 0 && strcmp(CHOICES[i], "viss") == 0)
        {
            printf("[Running VISS module test]\n");
            int app_modules_viss_test(int argc, char* argv[]);

            status = app_modules_viss_test(argc, argv);

        }

        else if(status == 0 && strcmp(CHOICES[i], "pyramid") == 0)
        {
            printf("[Running PYRAMID module test]\n");
            int app_modules_pyramid_test(int argc, char* argv[]);

            status = app_modules_pyramid_test(argc, argv);

        }

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)

        else if(status == 0 && strcmp(CHOICES[i], "dof") == 0)
        {
            printf("[Running DOF module test]\n");
            int app_modules_dof_test(int argc, char* argv[]);

            status = app_modules_dof_test(argc, argv);

        }
#endif

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
        else if(status == 0 && strcmp(CHOICES[i], "dof-viz") == 0)
        {
            printf("[Running DOF Viz module test]\n");
            int app_modules_dof_viz_test(int argc, char* argv[]);

            status = app_modules_dof_viz_test(argc, argv);

        }
#endif

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
        else if(status == 0 && strcmp(CHOICES[i], "sde") == 0)
        {
            printf("[Running SDE module test]\n");
            int app_modules_sde_test(int argc, char* argv[]);

            status = app_modules_sde_test(argc, argv);

        }
#endif

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
        else if(status == 0 && strcmp(CHOICES[i], "sde-viz") == 0)
        {
            printf("[Running SDE Viz module test]\n");
            int app_modules_sde_viz_test(int argc, char* argv[]);

            status = app_modules_sde_viz_test(argc, argv);

        }
#endif

        else if(status == 0 && strcmp(CHOICES[i], "viss-ldc-msc") == 0)
        {
            printf("[Running VISS-LDC-MSC module test]\n");
            int app_modules_viss_msc_ldc_test(int argc, char* argv[]);

            status = app_modules_viss_msc_ldc_test(argc, argv);

        }
    }

    printf("All tests complete!\n");

    appDeInit();

    return status;
}
