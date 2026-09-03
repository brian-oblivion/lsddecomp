#include "common.h"
#include "Entity.h"

/* Data tables reached with a raw pointer by this unit's mood-dispatch
 * handlers -- same convention as Entity_d.c/Entity_c.c's own D_80089E50/
 * D_80089E38/etc externs (separate local view per translation unit, not
 * shared via the header). */
extern u8 D_80089C64[];
extern u8 D_80089C7C[];
extern u8 D_80089CD0[];
extern u8 D_80089CDC[];
extern u8 D_80089D90[];
extern u8 D_80089DE4[];
extern u8 D_80089E38[];

/* Forward declarations: both are defined later in this file (in ROM
 * order), but func_80063874 and func_80063BC0 call them before their own
 * definitions appear -- same convention as Entity_d.c's own forward calls. */
void func_80063C84(EntityMoodHandlerArg *out);
void func_80063CAC(EntityMoodHandlerArg *out);

void func_800634A8(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk44 == 0 && this->unkFC == 0) {
        if (rand() % 3 != 0) {
            this->unk44 = (rand() & 1) ? 0xB : 0xC;
        } else {
            this->methods->slot16C(this);
            this->methods->slotC4(this, -0x5000, 0);
            rand();
        }
    }
    if (this->unk44 == 0xC) {
        func_8001EACC(this, this->unk94, 1, 0, 0);
        if (this->unkFC == 0x14) {
            out->unk1C = 0x12;
            out->unk10 = 0;
            out->unk30 = 3;
            this->unk94->methods->slot130(this->unk94, 1);
        }
        if (this->unkFC >= 0x15) {
            this->methods->slotC4(this, -0x28, 0);
        }
        if (this->unkFC == 0x28) {
            this->methods->slot30(this, 0xA);
        }
    } else if (this->unk44 == 0xB) {
        this->methods->slot130(this);
        if (this->methods->slot144(this, this->unk94) < 0x200) {
            if (rand() % 3 != 0) {
                this->methods->slot160(this);
            } else {
                this->methods->slot16C(this);
            }
        }
    }
}

void func_800636E4(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk84 % 15 == 0) {
        out->unk10 = 0;
        out->unk1C = 0xC;
        out->unk20 = 2;
    }
    if (this->unkFC == this->unk80) {
        out->unk1C = -2;
        this->methods->slot16C(this);
        this->unk44 = 1;
    }
}

void func_80063784(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (this->unk84 < 0x28) {
        out->unk1C = 0xC;
        out->unk20 = -2;
        out->unk44 = 5;
        out->unk48 = -1;
        return;
    }
    if (this->unk84 == 0x28) {
        out->unk1C = -2;
        out->unk44 = -2;
        return;
    }
    if (this->unk84 == 0x2D) {
        out->unk30 = 0x12;
        out->unk34 = 1;
        return;
    }
    if (this->unk84 == 0x40) {
        out->unk1C = 7;
        return;
    }
    if (this->unk84 == 0x59) {
        this->methods->slot16C(this);
        this->unk44 = 1;
    }
}

void func_80063874(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk44 == 0) {
        if (this->unk84 == 5) {
            func_80063C84(out);
        }
        if (this->unkFC == this->unk80) {
            this->methods->slot130(this);
            this->unk44 = 0xA;
            this->unkFC = -1;
        }
    } else if (this->unk44 == 0xA) {
        if (this->unkFC < 0xA) {
            this->methods->slot44(this, 0, D_80089C64);
            if (this->unk94->methods->slot100(this->unk94) != 0) {
                func_80063C84(out);
                this->unk44 = 0xC;
                this->unkFC = -1;
            }
        } else {
            this->unk94->methods->slot130(this->unk94, 1);
            this->unk44 = 0xB;
            this->unkFC = -1;
        }
    } else if (this->unk44 == 0xB) {
        func_8001EACC(this, this->unk94, 1, 0, 0);
        if (this->unkFC < 0x1E) {
            this->methods->slotC4(this, -0xA, 0);
        } else {
            func_80063C84(out);
            if (func_8005D108(this, NULL, NULL, (void *)0x1E, 0) != NULL) {
                this->unk100->methods->slotD4(this->unk100, this->unk50, 7, 0);
            }
            this->unk44 = 0xD;
            this->unkFC = -1;
        }
    } else if (this->unk44 == 0xD) {
        if (this->unkFC < 0x5A) {
            if (this->unkFC == 0x1E) {
                if (func_8005D108(this, NULL, NULL, (void *)0xA, 0) != NULL) {
                    this->unk100->methods->slotD8(this->unk100, this->unk50, 0, 0);
                }
            }
            this->unk94->methods->slot44(this->unk94, 0, D_80089CD0);
        } else {
            func_80063CAC(out);
            this->unk94->methods->slot44(this->unk94, 1, D_80089C7C);
            this->methods->slot30(this, (rand() % 5 != 0) ? 0xA : 0xC);
            this->unk44 = 0xE;
        }
    } else if (this->unk44 == 0xC) {
        if (this->unkFC < 0xA) {
            this->methods->slot44(this, 0, D_80089CDC);
        } else {
            func_80063CAC(out);
            this->methods->slot16C(this);
            this->unk44 = 1;
        }
    }
}

void func_80063BC0(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC < 0xA) {
        this->methods->slot130(this);
    } else if (this->unkFC == 0xA) {
        this->methods->slot12C(this);
    }
    if (this->unk84 == 0xA) {
        func_80063CAC(out);
    }
    if (this->unkFC == this->unk80 + 0xA) {
        this->methods->slot16C(this);
        this->unk44 = 1;
    }
}

void func_80063C84(EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    out->unk1C = 7;
    out->unk20 = -2;
    out->unk30 = 7;
    out->unk34 = -2;
    out->unk44 = 7;
    out->unk48 = -2;
}

void func_80063CAC(EntityMoodHandlerArg *out) {
    out->unk1C = 0x12;
    out->unk10 = 0;
    out->unk30 = 3;
    out->unk44 = 3;
}

void func_80063CC8(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 == 0) {
        out->unk1C = 0x12;
    }
    if (out->unk4 >= this->unk80 - 1) {
        out->unk4 = -1;
    }
}

void func_80063D40(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 == this->unk80 / 2) {
        out->unk1C = 0x12;
    }
    if (out->unk4 >= this->unk80 - 1) {
        out->unk4 = -1;
    }
}

void func_80063DC8(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0x14) {
        out->unk1C = 0x12;
        out->unk10 = 0;
        out->unk30 = 3;
        return;
    }
    if (this->unkFC == this->unk80) {
        this->methods->slot16C(this);
        this->unk44 = 1;
        if (rand() & 1) {
            this->methods->slot30(this, 0xB);
        }
    }
}

void func_80063E68(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 == 0) {
        out->unk1C = (rand() & 1) ? 2 : 1;
        out->unk20 = 2;
    }
}

void func_80063ED4(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        if (func_8005D108(this, NULL, NULL, (void *)5, 0) != NULL) {
            if (rand() & 1) {
                this->methods->slotBC(this, D_80089D90);
            }
            this->unk100->methods->slotD4(this->unk100, this->unk50, 0, 0);
        }
    } else {
        if (this->unk84 == 0) {
            do {
                this->unk88 = this->methods->slot134(this, this->unk88, 0);
                this->unk84 += 1;
            } while (this->unk84 < 0x18);
        }
    }
    if (this->unk84 >= 0x19) {
        this->methods->slotC4(this, -0x14, 0);
        this->unk94->methods->slot130(this->unk94, 1);
    }
    if (this->unkFC == 0x32) {
        this->methods->slot30(this, 0xA);
    } else if (this->unkFC == 0xC) {
        out->unk10 = 0;
        out->unk1C = 0x15;
    }
    this->methods->slot48(this, 1, D_80089DE4);
}

void func_80064078(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk7C == 0) {
        if (this->unkF4 != 0) {
            func_8001EACC(this, this->unk94, 1, 0, 0);
            this->methods->slot128(this, 1);
            this->unk94->methods->slot130(this->unk94, 1);
        } else if (this->unk84 == 0) {
            do {
                this->unk88 = this->methods->slot134(this, this->unk88, 0);
                this->unk84 += 1;
            } while (this->unk84 < 0x18);
        }
    } else {
        if (this->unk84 == 0) {
            out->unk10 = 0;
            out->unk1C = 0x16;
        } else if (this->unk84 == this->unk80 - 1) {
            out->unk10 = 0;
            out->unk30 = 0x12;
            this->methods->slot30(this, 0xA);
        }
    }
    this->methods->slot48(this, 1, D_80089DE4);
}

void func_800641C0(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 3;
    }
    if (this->unkFC == this->unk80) {
        this->methods->slot128(this, 1);
    }
    if (this->unk7C == 1) {
        this->methods->slotC4(this, -0x80, 1);
    }
}

void func_80064294(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        if (this->unk94->methods->slot200(this->unk94) != 7) {
            this->unk44 = 0xB;
        }
    }
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % 10 == 0) {
        out->unk1C = 0xE;
    }
    if (this->unkFC == this->unk80) {
        this->methods->slot128(this, 1);
        if (this->unk44 != 0) {
            if ((rand() & 1) == 0) {
                this->methods->slot48(this, 1, D_80089E38);
                this->methods->slotCC(this, 0x800, 0);
            }
        }
        if (rand() % 3 == 0) {
            this->methods->slot44(this, 0, D_80089C7C);
        }
    }
    if (this->unk7C != 0) {
        this->methods->slotC4(this, -0x80, 1);
    }
}

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_80064450);

INCLUDE_ASM("asm/nonmatchings/Entity_f", func_800644E8);
