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




#ifndef __gc_hal_user_hardware_h_
#define __gc_hal_user_hardware_h_


#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
********************************** Structures **********************************
\******************************************************************************/

/* FilterBlt information. */
typedef struct _gcsFILTER_BLIT_ARRAY
{
    gceFILTER_TYPE              filterType;
    gctUINT8                    kernelSize;
    gctUINT32                   scaleFactor;
    gctUINT16_PTR               kernelArray;
    gctBOOL                     kernelChanged;
}
gcsFILTER_BLIT_ARRAY;

typedef gcsFILTER_BLIT_ARRAY *  gcsFILTER_BLIT_ARRAY_PTR;

typedef enum
{
    gcvVAA_NONE,
    gcvVAA_COVERAGE_16,
    gcvVAA_COVERAGE_8,
}
gceVAA;

/* gcoHARDWARE object. */
struct _gcoHARDWARE
{
    /* Object. */
    gcsOBJECT                   object;

    /* Pointer to the gcoHAL object. */
    gcoHAL                      hal;

    /* Pointer to the gcoOS object. */
    gcoOS                       os;

    /* Command buffer. */
    gcoBUFFER                   buffer;

    /* Context buffer. */
    gcoCONTEXT                  context;

    /* Event queue. */
    gcoQUEUE                    queue;

    /* Chip characteristics. */
    gceCHIPMODEL                chipModel;
    gctUINT32                   chipRevision;
    gctUINT32                   chipFeatures;
    gctUINT32                   chipMinorFeatures;
    gctUINT32                   chipMinorFeatures1;
    gctUINT32                   streamCount;
    gctUINT32                   registerMax;
    gctUINT32                   threadCount;
    gctUINT32                   shaderCoreCount;
    gctUINT32                   vertexCacheSize;
    gctUINT32                   vertexOutputBufferSize;
    gctINT                      needStriping;

    /* Big endian */
    gctBOOL                     bigEndian;

    /* API type. */
    gceAPI                      api;

    /* Temporary buffer parameters. */
    struct _gcsSURF_INFO        tempBuffer;

    /* Filter blit. */
    gceFILTER_TYPE              loadedFilterType;
    gctUINT8                    loadedKernelSize;
    gctUINT32                   loadedScaleFactor;

    gceFILTER_TYPE              newFilterType;
    gctUINT8                    newHorKernelSize;
    gctUINT8                    newVerKernelSize;

    gctBOOL                     horUserFilterPass;
    gctBOOL                     verUserFilterPass;

    gcsFILTER_BLIT_ARRAY        horSyncFilterKernel;
    gcsFILTER_BLIT_ARRAY        verSyncFilterKernel;

    gcsFILTER_BLIT_ARRAY        horBlurFilterKernel;
    gcsFILTER_BLIT_ARRAY        verBlurFilterKernel;

    gcsFILTER_BLIT_ARRAY        horUserFilterKernel;
    gcsFILTER_BLIT_ARRAY        verUserFilterKernel;

    /* Depth mode. */
    gceDEPTH_MODE               depthMode;
    gctBOOL                     depthOnly;

    /* Maximum depth value. */
    gctUINT32                   maxDepth;
    gctBOOL                     earlyDepth;

    /* Stencil mask. */
    gctBOOL                     stencilEnabled;
    gctUINT32                   stencilMode;
    gctBOOL                     stencilKeepFront[3];
    gctBOOL                     stencilKeepBack[3];

    /* Texture sampler modes. */
    gctUINT32                   samplerMode[16];
    gctUINT32                   samplerLOD[12];

    /* Stall before rendingering triangles. */
    gctBOOL                     stallPrimitive;

    /* Tile status information. */
    gctUINT32                   memoryConfig;
    gctBOOL                     paused;
    gctBOOL                     cacheDirty;
    gctBOOL                     targetDirty;
    gcsSURF_INFO_PTR            currentTarget;
    gcsSURF_INFO_PTR            currentDepth;
    gctBOOL                     inFlush;
    gctUINT32                   physicalTileColor;
    gctUINT32                   physicalTileDepth;

    /* Anti-alias mode. */
    gctUINT32                   sampleMask;
    gctUINT32                   sampleEnable;
    gcsSAMPLES                  samples;
    gceVAA                      vaa;
    struct
    {
        gctUINT8                x;
        gctUINT8                y;
    }                           sampleCoords[4][4];
    gctUINT8                    jitterIndex[4][4];

    /* Dither. */
    gctUINT32                   dither[2];

    /* For bandwidth optimization. */
    gctBOOL                     alphaBlendEnable;
    gctUINT8                    colorWrite;
    gctBOOL                     colorCompression;
    gctUINT32                   destinationRead;

    /* Stall signal. */
    gctSIGNAL                   stallSignal;

    /***************************************************************************
    ** 2D states.
    */

    /* 2D hardware availability flag. */
    gctBOOL                     hw2DEngine;

    /* Software 2D force flag. */
    gctBOOL                     sw2DEngine;

    /* 2D hardware Pixel Engine 2.0 availability flag. */
    gctBOOL                     hw2DPE20;

    /* Byte write capability. */
    gctBOOL                     byteWrite;

    /* BitBlit rotation capability. */
    gctBOOL                     fullBitBlitRotation;

    /* FilterBlit rotation capability. */
    gctBOOL                     fullFilterBlitRotation;

    /* Need to shadow RotAngleReg? */
    gctBOOL                     shadowRotAngleReg;

    /* The shadow value. */
    gctUINT32                   rotAngleRegShadow;

    /* Pattern states. */
    gcoBRUSH_CACHE              brushCache;

    /* Temporary pattern table used for color convert. */
    gctUINT32_PTR               patternTable;
    gctBOOL                     patternTableProgram;
    gctUINT                     patternTableIndexCount;
    gctUINT                     patternTableFirstIndex;

    /* Mono colors needed for backward compatibility. */
    gctUINT32                   fgColor;
    gctUINT32                   bgColor;
    gctBOOL                     monoColorProgram;

    /* Global colors needed for backward compatibility. */
    gctUINT32                   globalSrcColor;
    gctUINT32                   globalTargetColor;

    /* Transparency states. */
    gctUINT32                   srcTransparency;
    gctUINT32                   dstTransparency;
    gctUINT32                   patTransparency;
    gctUINT32                   transparencyColor;
    gctBOOL                     transparencyColorProgram;

    /* Src configuration state. */
    gctUINT32                   srcConfig;

    /* 2D clipping rectangle. */
    gcsRECT                     clippingRect;

    /* Source rectangle. */
    gcsRECT                     sourceRect;

    /* Surface information. */
    struct _gcsSURF_INFO        sourceSurface;
    struct _gcsSURF_INFO        targetSurface;

    /* Temp surface for fast clear */
    gcoSURF                     tempSurface;
};

gceSTATUS
gcoHARDWARE_OptimizeBandwidth(
    IN gcoHARDWARE Hardware
    );

gceSTATUS
gcoHARDWARE_FlushL2Cache(
    IN gcoHARDWARE Hardware
    );

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_user_hardware_h_ */

