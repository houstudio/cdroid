/****************************************************************************
*
*    Copyright (c) 2005 - 2010 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************
*
*    Auto-generated file on 11/10/2010. Do not edit!!!
*
*****************************************************************************/




/*******************************************************************************
**	Profiler for Vivante HAL.
*/

#include "gc_hal_user_precomp.h"

#if VIVANTE_PROFILER

#define gcmWRITE_STRING(String) \
	do \
	{ \
		gceSTATUS status; \
		gctSIZE_T length; \
		gcmERR_BREAK(gcoOS_StrLen((gctSTRING) String, &length)); \
		gcmERR_BREAK(gcoOS_Write(Hal->os, \
								 Hal->profiler.file, \
								 length, String)); \
	} \
	while (gcvFALSE)

#define gcmPRINT_XML_COUNTER(Counter) \
	do \
	{ \
		char buffer[256]; \
		gctUINT offset = 0; \
		gceSTATUS status; \
		gcmERR_BREAK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), \
										&offset, \
										"<%s value=\"%d\"/>\n", \
										# Counter, \
										Hal->profiler.Counter)); \
		gcmWRITE_STRING(buffer); \
	} \
	while (gcvFALSE)

#define gcmPRINT_XML(Format, Value) \
	do \
	{ \
		char buffer[256]; \
		gctUINT offset = 0; \
		gceSTATUS status; \
		gcmERR_BREAK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), \
										&offset, \
										Format, \
										Value)); \
		gcmWRITE_STRING(buffer); \
	} \
	while (gcvFALSE)

gceSTATUS
gcoPROFILER_Initialize(
	IN gcoHAL Hal,
    IN gctFILE File
	)
{
    gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	if (File == gcvNULL)
	{
		return gcvSTATUS_OK;
	}

	gcmVERIFY_OK(gcoOS_ZeroMemory(&Hal->profiler, gcmSIZEOF(Hal->profiler)));

	Hal->profiler.file               = File;
	gcoOS_GetTime(&Hal->profiler.frameStart);
	Hal->profiler.frameStartTimeusec = Hal->profiler.frameStart;

    /* Success. */
    return gcvSTATUS_OK;
}

gceSTATUS
gcoPROFILER_Destroy(
	IN gcoHAL Hal
	)
{
    gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

    return gcvSTATUS_OK;
}

gceSTATUS
gcoPROFILER_Count(
	IN gcoHAL Hal,
	IN gctUINT32 Enum,
	IN gctINT Value
	)
{
#if PROFILE_HAL_COUNTERS
    gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	if (Hal->profiler.file != gcvNULL)
	{
		switch (Enum)
		{
		case GLINDEX_OBJECT:
			Hal->profiler.indexBufferNewObjectsAlloc   += Value;
			Hal->profiler.indexBufferTotalObjectsAlloc += Value;
			break;

		case GLINDEX_OBJECT_BYTES:
			Hal->profiler.indexBufferNewBytesAlloc   += Value;
			Hal->profiler.indexBufferTotalBytesAlloc += Value;
			break;

		case GLVERTEX_OBJECT:
			Hal->profiler.vertexBufferNewObjectsAlloc   += Value;
			Hal->profiler.vertexBufferTotalObjectsAlloc += Value;
			break;

		case GLVERTEX_OBJECT_BYTES:
			Hal->profiler.vertexBufferNewBytesAlloc   += Value;
			Hal->profiler.vertexBufferTotalBytesAlloc += Value;
			break;

		case GLTEXTURE_OBJECT:
			Hal->profiler.textureBufferNewObjectsAlloc   += Value;
			Hal->profiler.textureBufferTotalObjectsAlloc += Value;
			break;

		case GLTEXTURE_OBJECT_BYTES:
			Hal->profiler.textureBufferNewBytesAlloc   += Value;
			Hal->profiler.textureBufferTotalBytesAlloc += Value;
			break;

		default:
			break;
		}
	}
#endif

    return gcvSTATUS_OK;
}

gceSTATUS
gcoPROFILER_Shader(
    IN gcoHAL Hal,
    IN gcSHADER Shader
    )
{
#if PROFILE_SHADER_COUNTERS
    gctUINT16 alu = 0, tex = 0, i;

    gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	if (Hal->profiler.file != gcvNULL)
	{
	    /* Profile shader */
    	for (i = 0; i < Shader->codeCount; i++ )
    	{
        	switch (Shader->code[i].opcode)
        	{
			case gcSL_NOP:
				break;

			case gcSL_TEXLD:
				tex++;
				break;

			default:
				alu++;
				break;
			}
		}

    	gcmPRINT_XML("<InstructionCount value=\"%d\"/>\n", tex + alu);
		gcmPRINT_XML("<ALUInstructionCount value=\"%d\"/>\n", alu);
		gcmPRINT_XML("<TextureInstructionCount value=\"%d\"/>\n", tex);
		gcmPRINT_XML("<Attributes value=\"%lu\"/>\n", Shader->attributeCount);
		gcmPRINT_XML("<Uniforms value=\"%lu\"/>\n", Shader->uniformCount);
		gcmPRINT_XML("<Functions value=\"%lu\"/>\n", Shader->functionCount);
	}
#endif

    return gcvSTATUS_OK;
}

gceSTATUS
gcoPROFILER_ShaderVS(
    IN gcoHAL Hal,
    IN gctPOINTER Vs
    )
{
#if PROFILE_SHADER_COUNTERS
    gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	if (Hal->profiler.file != gcvNULL)
	{
		gcmWRITE_STRING("<VS>\n");
		gcoPROFILER_Shader(Hal, (gcSHADER) Vs);
		gcmWRITE_STRING("</VS>\n");
	}
#endif

    return gcvSTATUS_OK;
}

gceSTATUS
gcoPROFILER_ShaderFS(
    IN gcoHAL Hal,
    IN void* Fs
    )
{
#if PROFILE_SHADER_COUNTERS
    gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	if (Hal->profiler.file != gcvNULL)
	{
		gcmWRITE_STRING("<FS>\n");
		gcoPROFILER_Shader(Hal, (gcSHADER) Fs);
		gcmWRITE_STRING("</FS>\n");
	}
#endif

    return gcvSTATUS_OK;
}

gceSTATUS
gcoPROFILER_EndFrame(
    IN gcoHAL Hal
    )
{
#if	(PROFILE_HAL_COUNTERS || PROFILE_HW_COUNTERS)
	gcsHAL_INTERFACE iface;
    gceSTATUS status;
#endif

    gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	if (Hal->profiler.file == gcvNULL)
	{
		return gcvSTATUS_OK;
	}

#if PROFILE_HAL_COUNTERS
    gcmWRITE_STRING("<HALCounters>\n");

    gcmPRINT_XML_COUNTER(vertexBufferNewBytesAlloc);
    gcmPRINT_XML_COUNTER(vertexBufferTotalBytesAlloc);
    gcmPRINT_XML_COUNTER(vertexBufferNewObjectsAlloc);
    gcmPRINT_XML_COUNTER(vertexBufferTotalObjectsAlloc);

    gcmPRINT_XML_COUNTER(indexBufferNewBytesAlloc);
    gcmPRINT_XML_COUNTER(indexBufferTotalBytesAlloc);
    gcmPRINT_XML_COUNTER(indexBufferNewObjectsAlloc);
    gcmPRINT_XML_COUNTER(indexBufferTotalObjectsAlloc);

    gcmPRINT_XML_COUNTER(textureBufferNewBytesAlloc);
    gcmPRINT_XML_COUNTER(textureBufferTotalBytesAlloc);
    gcmPRINT_XML_COUNTER(textureBufferNewObjectsAlloc);
    gcmPRINT_XML_COUNTER(textureBufferTotalObjectsAlloc);

    gcmWRITE_STRING("</HALCounters>\n");

    /* Reset per-frame counters. */
    Hal->profiler.vertexBufferNewBytesAlloc   = 0;
    Hal->profiler.vertexBufferNewObjectsAlloc = 0;

    Hal->profiler.indexBufferNewBytesAlloc   = 0;
    Hal->profiler.indexBufferNewObjectsAlloc = 0;

    Hal->profiler.textureBufferNewBytesAlloc   = 0;
  	Hal->profiler.textureBufferNewObjectsAlloc = 0;
#endif

#if PROFILE_HW_COUNTERS
    /* gcvHAL_READ_ALL_PROFILE_REGISTERS. */
	iface.command = gcvHAL_READ_ALL_PROFILE_REGISTERS;

	/* Call the kernel. */
	status = gcoOS_DeviceControl(Hal->os,
								 IOCTL_GCHAL_INTERFACE,
								 &iface, gcmSIZEOF(iface),
								 &iface, gcmSIZEOF(iface));

	/* Verify result. */
	if (gcmNO_ERROR(status))
	{
	    gcmWRITE_STRING("<HWCounters>\n");

	    #define gcmCOUNTER(name)	iface.u.RegisterProfileData.counters.name

        gcmPRINT_XML("<read_64Byte value=\"%u\"/>\n",
        			 gcmCOUNTER(gpuTotalRead64BytesPerFrame));
        gcmPRINT_XML("<write_64Byte value=\"%u\"/>\n",
        			 gcmCOUNTER(gpuTotalWrite64BytesPerFrame));
        gcmPRINT_XML("<gpu_cycles value=\"%u\"/>\n",
        			 gcmCOUNTER(gpuCyclesCounter));
        gcmPRINT_XML("<pe_pixel_count_killed_by_color_pipe value=\"%u\"/>\n",
        			 gcmCOUNTER(pe_pixel_count_killed_by_color_pipe));
        gcmPRINT_XML("<pe_pixel_count_killed_by_depth_pipe value=\"%u\"/>\n",
        			 gcmCOUNTER(pe_pixel_count_killed_by_depth_pipe));
        gcmPRINT_XML("<pe_pixel_count_drawn_by_color_pipe value=\"%u\"/>\n",
					 gcmCOUNTER(pe_pixel_count_drawn_by_color_pipe));
        gcmPRINT_XML("<pe_pixel_count_drawn_by_depth_pipe value=\"%u\"/>\n",
					 gcmCOUNTER(pe_pixel_count_drawn_by_depth_pipe));
        gcmPRINT_XML("<ps_inst_counter value=\"%u\"/>\n",
					 gcmCOUNTER(ps_inst_counter));
        gcmPRINT_XML("<rendered_pixel_counter value=\"%u\"/>\n",
					 gcmCOUNTER(rendered_pixel_counter));
        gcmPRINT_XML("<vs_inst_counter value=\"%u\"/>\n",
					 gcmCOUNTER(vs_inst_counter));
        gcmPRINT_XML("<rendered_vertice_counter value=\"%u\"/>\n",
					 gcmCOUNTER(rendered_vertice_counter));
        gcmPRINT_XML("<vtx_branch_inst_counter value=\"%u\"/>\n",
					 gcmCOUNTER(vtx_branch_inst_counter));
        gcmPRINT_XML("<vtx_texld_inst_counter value=\"%u\"/>\n",
					 gcmCOUNTER(vtx_texld_inst_counter));
        gcmPRINT_XML("<pxl_branch_inst_counter value=\"%u\"/>\n",
					 gcmCOUNTER(pxl_branch_inst_counter));
        gcmPRINT_XML("<pxl_texld_inst_counter value=\"%u\"/>\n",
					 gcmCOUNTER(pxl_texld_inst_counter));
        gcmPRINT_XML("<pa_input_vtx_counter value=\"%u\"/>\n",
					 gcmCOUNTER(pa_input_vtx_counter));
        gcmPRINT_XML("<pa_input_prim_counter value=\"%u\"/>\n",
					 gcmCOUNTER(pa_input_prim_counter));
        gcmPRINT_XML("<pa_output_prim_counter value=\"%u\"/>\n",
					 gcmCOUNTER(pa_output_prim_counter));
        gcmPRINT_XML("<pa_depth_clipped_counter value=\"%u\"/>\n",
					 gcmCOUNTER(pa_depth_clipped_counter));
        gcmPRINT_XML("<pa_trivial_rejected_counter value=\"%u\"/>\n",
					 gcmCOUNTER(pa_trivial_rejected_counter));
        gcmPRINT_XML("<pa_culled_counter value=\"%u\"/>\n",
					 gcmCOUNTER(pa_culled_counter));
        gcmPRINT_XML("<se_culled_triangle_count value=\"%u\"/>\n",
					 gcmCOUNTER(se_culled_triangle_count));
        gcmPRINT_XML("<se_culled_lines_count value=\"%u\"/>\n",
					 gcmCOUNTER(se_culled_lines_count));
        gcmPRINT_XML("<ra_valid_pixel_count value=\"%u\"/>\n",
					 gcmCOUNTER(ra_valid_pixel_count));
        gcmPRINT_XML("<ra_total_quad_count value=\"%u\"/>\n",
					 gcmCOUNTER(ra_total_quad_count));
        gcmPRINT_XML("<ra_valid_quad_count_after_early_z value=\"%u\"/>\n",
					 gcmCOUNTER(ra_valid_quad_count_after_early_z));
        gcmPRINT_XML("<ra_total_primitive_count value=\"%u\"/>\n",
					 gcmCOUNTER(ra_total_primitive_count));
        gcmPRINT_XML("<ra_pipe_cache_miss_counter value=\"%u\"/>\n",
					 gcmCOUNTER(ra_pipe_cache_miss_counter));
        gcmPRINT_XML("<ra_prefetch_cache_miss_counter value=\"%u\"/>\n",
					 gcmCOUNTER(ra_prefetch_cache_miss_counter));
        gcmPRINT_XML("<ra_eez_culled_counter value=\"%u\"/>\n",
					 gcmCOUNTER(ra_eez_culled_counter));
        gcmPRINT_XML("<tx_total_bilinear_requests value=\"%u\"/>\n",
					 gcmCOUNTER(tx_total_bilinear_requests));
        gcmPRINT_XML("<tx_total_trilinear_requests value=\"%u\"/>\n",
					 gcmCOUNTER(tx_total_trilinear_requests));
        gcmPRINT_XML("<tx_total_discarded_texture_requests value=\"%u\"/>\n",
					 gcmCOUNTER(tx_total_discarded_texture_requests));
        gcmPRINT_XML("<tx_total_texture_requests value=\"%u\"/>\n",
					 gcmCOUNTER(tx_total_texture_requests));
        gcmPRINT_XML("<tx_mem_read_count value=\"%u\"/>\n",
					 gcmCOUNTER(tx_mem_read_count));
        gcmPRINT_XML("<tx_mem_read_in_8B_count value=\"%u\"/>\n",
					 gcmCOUNTER(tx_mem_read_in_8B_count));
        gcmPRINT_XML("<tx_cache_miss_count value=\"%u\"/>\n",
					 gcmCOUNTER(tx_cache_miss_count));
        gcmPRINT_XML("<tx_cache_hit_texel_count value=\"%u\"/>\n",
					 gcmCOUNTER(tx_cache_hit_texel_count));
        gcmPRINT_XML("<tx_cache_miss_texel_count value=\"%u\"/>\n",
					 gcmCOUNTER(tx_cache_miss_texel_count));
        gcmPRINT_XML("<mc_total_read_req_8B_from_pipeline value=\"%u\"/>\n",
					 gcmCOUNTER(mc_total_read_req_8B_from_pipeline));
        gcmPRINT_XML("<mc_total_read_req_8B_from_IP value=\"%u\"/>\n",
					 gcmCOUNTER(mc_total_read_req_8B_from_IP));
        gcmPRINT_XML("<mc_total_write_req_8B_from_pipeline value=\"%u\"/>\n",
					 gcmCOUNTER(mc_total_write_req_8B_from_pipeline));
        gcmPRINT_XML("<hi_axi_cycles_read_request_stalled value=\"%u\"/>\n",
					 gcmCOUNTER(hi_axi_cycles_read_request_stalled));
        gcmPRINT_XML("<hi_axi_cycles_write_request_stalled value=\"%u\"/>\n",
					 gcmCOUNTER(hi_axi_cycles_write_request_stalled));
        gcmPRINT_XML("<hi_axi_cycles_write_data_stalled value=\"%u\"/>\n",
					 gcmCOUNTER(hi_axi_cycles_write_data_stalled));

	    gcmWRITE_STRING("</HWCounters>\n");
	}
#endif

	/* Success. */
    return gcvSTATUS_OK;
}

#endif /* VIVANTE_PROFILER */

