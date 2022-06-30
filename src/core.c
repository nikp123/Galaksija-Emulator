#include <stdio.h>
#include <stdlib.h>

#include "util/file.h"

#include "shared.h"
#include "graphics.h"
#include "dos.h"

bool galaxy_run_frame(galaxy_state *state, void *framebuffer) {
    Z80ExecuteTStates(&state->context, state->config.cpu_speed/state->config.framerate);

    if(galaxy_draw(state, framebuffer) == false) {
        return false;
    }

    /**
     * This line of code is so important. DO NOT TOUCHY!!!
     *
     * It basically triggers the screen draw routine that HANDLES the
     * framebuffer. Otherwise, you'll have trouble rendering more than
     * 16 lines on the screen. You're welcome.
     **/
    Z80INT(&state->context, 0x00);

    return true;
}

void galaxy_trigger_nmi(galaxy_state *state) {
    Z80NMI(&state->context);
}

byte galaxy_mem_read(galaxy_state *state, ushort address) {
    return state->memory[address];
}

void galaxy_mem_write(galaxy_state *state, ushort address, byte data) {
    if(address >= GALAXY_RAM_ADDR_START && address < GALAXY_RAM_ADDR_END) {
        state->memory[address&0xffff] = data;
    }
}

byte galaxy_io_read(galaxy_state *state, ushort address) {
    return 0;
}

void galaxy_io_write(galaxy_state *state, ushort address, byte data) {
}

bool galaxy_load_state(galaxy_state *state, const char *filename) {
    FILE *f;

    if((f=fopen(filename, "rb"))==NULL) {
        state->error = GALAXY_STATE_LOAD_FILE_FAILED;
        return false;
    }

    // DOS savestate
    if(util_file_size(filename) == GALAXY_DOS_STATE_SIZE) {
        Z80_RegsDOS DOS_R;
        fread(&DOS_R, 1, sizeof(Z80_RegsDOS), f);
        fread(&state->memory[GALAXY_RAM_ADDR_START], 1, GALAXY_RAM_SIZE, f);

        // Convert registers Now !
        state->context.R1.wr.AF = DOS_R.AF.W.l;
        state->context.R1.wr.BC = DOS_R.BC.W.l;
        state->context.R1.wr.DE = DOS_R.DE.W.l;
        state->context.R1.wr.HL = DOS_R.HL.W.l;
        state->context.R1.wr.IX = DOS_R.IX.W.l;
        state->context.R1.wr.IY = DOS_R.IY.W.l;
        state->context.PC       = DOS_R.PC.W.l;
        state->context.R1.wr.SP = DOS_R.SP.W.l;
        state->context.R2.wr.AF = DOS_R.AF2.W.l;
        state->context.R2.wr.BC = DOS_R.BC2.W.l;
        state->context.R2.wr.DE = DOS_R.DE2.W.l;
        state->context.R2.wr.HL = DOS_R.HL2.W.l;
        // instruction modes should be checked for equivalency, but it's pretty safe to assume they are
        state->context.IM       = DOS_R.IM;
        state->context.IFF1     = DOS_R.IFF1;
        state->context.IFF2     = DOS_R.IFF2;
        state->context.I        = DOS_R.I;
        state->context.R        = (DOS_R.R&127)|(DOS_R.R2&128);
    } else {
        // i386-assumed save state (because the original developer didn't bother)

        // pair AF,BC,DE,HL,IX,IY,PC,SP;       /* Main registers      */
        // pair AF1,BC1,DE1,HL1;               /* Shadow registers    */
        util_fread_little_endian(&state->context.R1.wr.AF, sizeof(uint16_t), 1, f);
        util_fread_little_endian(&state->context.R1.wr.BC, sizeof(uint16_t), 1, f);
        util_fread_little_endian(&state->context.R1.wr.DE, sizeof(uint16_t), 1, f);
        util_fread_little_endian(&state->context.R1.wr.HL, sizeof(uint16_t), 1, f);
        util_fread_little_endian(&state->context.R1.wr.IX, sizeof(uint16_t), 1, f);
        util_fread_little_endian(&state->context.R1.wr.IY, sizeof(uint16_t), 1, f);
        util_fread_little_endian(&state->context.PC,       sizeof(uint16_t), 1, f);
        util_fread_little_endian(&state->context.R1.wr.SP, sizeof(uint16_t), 1, f);
        util_fread_little_endian(&state->context.R2.wr.AF, sizeof(uint16_t), 1, f);
        util_fread_little_endian(&state->context.R2.wr.BC, sizeof(uint16_t), 1, f);
        util_fread_little_endian(&state->context.R2.wr.DE, sizeof(uint16_t), 1, f);
        util_fread_little_endian(&state->context.R2.wr.HL, sizeof(uint16_t), 1, f);

        // byte IFF,I;                         /* Interrupt registers */
        // byte R;                             /* Refresh register    */
        uint8_t iff = 0;
        state->context.IM = 0;
        util_fread_little_endian(&iff, sizeof(byte), 1, f);

        // yes i know both can be triggered, but that's a flaw of the previous emulator
        state->context.IFF1 = (iff & 0x1) ? 1 : 0;
        if(iff & 0x2)
            state->context.IM = 1;
        if(iff & 0x4)
            state->context.IM = 2;

        util_fread_little_endian(&state->context.I, sizeof(byte), 1, f);
        util_fread_little_endian(&state->context.R, sizeof(byte), 1, f);
        fseek(f, sizeof(uint8_t), SEEK_CUR);               /* padding */

        // int IPeriod,ICount; /* Set IPeriod to number of CPU cycles */
        //                     /* between calls to LoopZ80()          */
        // int IBackup;        /* Private, don't touch                */
        fseek(f, sizeof(uint32_t)*3, SEEK_CUR);               /* skip */

        // word IRequest;      /* Set to address of pending IRQ       */
        fseek(f, sizeof(uint16_t)*1, SEEK_CUR);               /* skip */

        // byte IAutoReset;    /* Set to 1 to autom. reset IRequest   */
        // byte TrapBadOps;    /* Set to 1 to warn of illegal opcodes */
        fseek(f, sizeof(uint8_t)*2, SEEK_CUR);                /* skip */

        // word Trap;          /* Set Trap to address to trace from   */
        fseek(f, sizeof(uint16_t)*1, SEEK_CUR);               /* skip */
        // byte Trace;         /* Set Trace=1 to start tracing        */
        fseek(f, sizeof(uint8_t)*1, SEEK_CUR);                /* skip */
        fseek(f, 1, SEEK_CUR);                             /* padding */

        // void *User;         /* Arbitrary user data (ID,RAM*,etc.)  */
        fseek(f, sizeof(uint32_t), SEEK_CUR);                 /* skip */

        // read memory
        fread(&state->memory[GALAXY_RAM_ADDR_START], 1, GALAXY_RAM_SIZE, f);
    }
    fclose(f);

    return true;
}

bool galaxy_save_state(galaxy_state *state, const char *filename) {
    FILE *f;

    if((f=fopen(filename, "wb"))==NULL) {
        state->error = GALAXY_STATE_SAVE_FILE_FAILED;
        return false;
    }
    // i386-assumed save state (because the original developer didn't bother)

    // pair AF,BC,DE,HL,IX,IY,PC,SP;       /* Main registers      */
    // pair AF1,BC1,DE1,HL1;               /* Shadow registers    */
    util_fwrite_little_endian(&state->context.R1.wr.AF, sizeof(uint16_t), 1, f);
    util_fwrite_little_endian(&state->context.R1.wr.BC, sizeof(uint16_t), 1, f);
    util_fwrite_little_endian(&state->context.R1.wr.DE, sizeof(uint16_t), 1, f);
    util_fwrite_little_endian(&state->context.R1.wr.HL, sizeof(uint16_t), 1, f);
    util_fwrite_little_endian(&state->context.R1.wr.IX, sizeof(uint16_t), 1, f);
    util_fwrite_little_endian(&state->context.R1.wr.IY, sizeof(uint16_t), 1, f);
    util_fwrite_little_endian(&state->context.PC,       sizeof(uint16_t), 1, f);
    util_fwrite_little_endian(&state->context.R1.wr.SP, sizeof(uint16_t), 1, f);
    util_fwrite_little_endian(&state->context.R2.wr.AF, sizeof(uint16_t), 1, f);
    util_fwrite_little_endian(&state->context.R2.wr.BC, sizeof(uint16_t), 1, f);
    util_fwrite_little_endian(&state->context.R2.wr.DE, sizeof(uint16_t), 1, f);
    util_fwrite_little_endian(&state->context.R2.wr.HL, sizeof(uint16_t), 1, f);

    // byte IFF,I;                         /* Interrupt registers */
    // byte R;                             /* Refresh register    */
    uint8_t iff = 0;
    switch(state->context.IM) {
        case 1: iff |= 0x2; break;
        case 2: iff |= 0x4; break;
        default: break;
    }
    if(state->context.IFF1) iff |= 0x1;

    util_fwrite_little_endian(&iff, sizeof(byte), 1, f);

    util_fwrite_little_endian(&state->context.I, sizeof(byte), 1, f);
    util_fwrite_little_endian(&state->context.R, sizeof(byte), 1, f);
    fseek(f, sizeof(uint8_t), SEEK_CUR);               /* padding */

    // int IPeriod,ICount; /* Set IPeriod to number of CPU cycles */
    //                     /* between calls to LoopZ80()          */
    // int IBackup;        /* Private, don't touch                */
    fseek(f, sizeof(uint32_t)*3, SEEK_CUR);               /* skip */

    // word IRequest;      /* Set to address of pending IRQ       */
    fseek(f, sizeof(uint16_t)*1, SEEK_CUR);               /* skip */

    // byte IAutoReset;    /* Set to 1 to autom. reset IRequest   */
    // byte TrapBadOps;    /* Set to 1 to warn of illegal opcodes */
    fseek(f, sizeof(uint8_t)*2, SEEK_CUR);                /* skip */

    // word Trap;          /* Set Trap to address to trace from   */
    fseek(f, sizeof(uint16_t)*1, SEEK_CUR);               /* skip */
    // byte Trace;         /* Set Trace=1 to start tracing        */
    fseek(f, sizeof(uint8_t)*1, SEEK_CUR);                /* skip */
    fseek(f, 1, SEEK_CUR);                             /* padding */

    // void *User;         /* Arbitrary user data (ID,RAM*,etc.)  */
    fseek(f, sizeof(uint32_t), SEEK_CUR);                 /* skip */

    // write memory
    fwrite(&state->memory[GALAXY_RAM_ADDR_START], 1, GALAXY_RAM_SIZE, f);

    fclose(f);

    return true;
}

