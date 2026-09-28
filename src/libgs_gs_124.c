/*
 * libgs_gs_124 -- Sony's libgs module gs_124: GsSetWorkBase, which sets
 * GsOUT_PACKET_P, the cursor libgs writes GPU packets through.
 *
 * Sony code carried as C. gs_124 never placed as a linked object, so its one
 * function is compiled here instead; the prototype and the global are
 * LIBGS.H's own, declared locally because no shared header carries a Sony
 * prototype.
 */
#include "common.h"

/* LIBGS.H: typedef unsigned char PACKET; extern PACKET *GsOUT_PACKET_P; */
extern unsigned char *GsOUT_PACKET_P;

/* LIBGS.H: void GsSetWorkBase(PACKET *outpacketp); */
void GsSetWorkBase(unsigned char *outpacketp) {
    GsOUT_PACKET_P = outpacketp;
}
