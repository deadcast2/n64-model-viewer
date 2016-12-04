#include <nusys.h>

#define SCREEN_WD 320
#define SCREEN_HT 240
#define GFX_GLIST_LEN 2048

struct ProjectionMatrix {
  Mtx projection;
  Mtx rotation;
  Mtx translation;
};

struct ProjectionMatrix projectionMatrix;
Gfx *glistp;
Gfx gfx_glist[GFX_GLIST_LEN];
u16 *perspNormal;

static Vtx vertices[] = {
    { -1, -1, 0, 0, 0, 0, 0xff, 0, 0, 0 },
    { 1, -1, 0, 0, 0, 0, 0, 0xff, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0, 0xff, 0 }
};

static Vp viewPort = {
    SCREEN_WD * 2, SCREEN_HT * 2, G_MAXZ / 2, 0,
    SCREEN_WD * 2, SCREEN_HT * 2, G_MAXZ / 2, 0,
};

Gfx rspState[] = {
  gsSPViewport(&viewPort),
  gsSPClearGeometryMode(0xFFFFFFFF),
  gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK),
  gsSPTexture(0, 0, 0, 0, G_OFF),
  gsSPEndDisplayList(),
};

Gfx rdpState[] = {
  gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
  gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
  gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, SCREEN_WD,SCREEN_HT),
  gsDPSetColorDither(G_CD_BAYER),
  gsSPEndDisplayList(),
};

void rcpInit() {
  gSPSegment(glistp++, 0, 0x0);
  gSPDisplayList(glistp++, OS_K0_TO_PHYSICAL(rspState));
  gSPDisplayList(glistp++, OS_K0_TO_PHYSICAL(rdpState));
}

void clearFramBuffer() {
    gDPSetDepthImage(glistp++, OS_K0_TO_PHYSICAL(nuGfxZBuffer));
    gDPSetCycleType(glistp++, G_CYC_FILL);
    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
    OS_K0_TO_PHYSICAL(nuGfxZBuffer));
    gDPSetFillColor(glistp++, (GPACK_ZDZ(G_MAXFBZ, 0) << 16 |
        GPACK_ZDZ(G_MAXFBZ, 0)));
    gDPFillRectangle(glistp++, 0, 0, SCREEN_WD - 1, SCREEN_HT - 1);
    gDPPipeSync(glistp++);

    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
        osVirtualToPhysical(nuGfxCfb_ptr));
    gDPSetFillColor(glistp++, (GPACK_RGBA5551(0, 0, 0, 1) << 16 |
        GPACK_RGBA5551(0, 0, 0, 1)));
    gDPFillRectangle(glistp++, 0, 0, SCREEN_WD - 1, SCREEN_HT - 1);
    gDPPipeSync(glistp++);
}

void setupTriangle(struct ProjectionMatrix* matrix) {
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(matrix->projection)),
        G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(matrix->translation)),
        G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(matrix->rotation)),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gSPVertex(glistp++, &(vertices[0]), 3, 0);
    gDPPipeSync(glistp++);
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);
    gDPSetRenderMode(glistp++, G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
    gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
    gSPSetGeometryMode(glistp++, G_SHADE | G_SHADING_SMOOTH);
    gSP1Triangle(glistp++, 0, 1, 2, 0);
}

void createDisplayList(int theta) {
    glistp = gfx_glist;
    rcpInit();
    clearFramBuffer();

    guPerspective(&projectionMatrix.projection,
        &perspNormal,
        60.0F, SCREEN_WD / SCREEN_HT,
        1.0F, 100.0F, 1.0F);

    guTranslate(&projectionMatrix.translation, 0.0F, 0.0F, -5.0F);
    guRotate(&projectionMatrix.rotation, theta, 0.0F, 1.0F, 0.0F);

    setupTriangle(&projectionMatrix);
    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);

    nuGfxTaskStart(gfx_glist, (s32)(glistp - gfx_glist) * sizeof (Gfx),
        NU_GFX_UCODE_F3DEX , NU_SC_SWAPBUFFER);

    nuDebConTextPos(0, 12, 5);
    nuDebConCPuts(0, "A triangle. Wow.");
    nuDebConDisp(NU_SC_SWAPBUFFER);
}

void gfxCallback(int pendingGfx) {
    static int theta = 0;

    if(pendingGfx < 1) {
        createDisplayList(theta);
        theta += 1;
    }
}

void mainproc() {
  nuGfxInit();
  while(1) {
    nuGfxFuncSet((NUGfxFunc)gfxCallback);
    nuGfxDisplayOn();
  }
}
