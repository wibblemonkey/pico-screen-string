@ -----------------------------------------------------------------------------
@ ZX0 decoder (ARM Thumb port of the Z80 "Turbo" decoder by Einar Saukas & introspec)
@ Targets Cortex-M0+ (RP2040 / Raspberry Pi Pico); also valid on Cortex-M33.
@ -----------------------------------------------------------------------------
@ C prototype:  uint8_t *dzx0_turbo(const uint8_t *src, uint8_t *dst);
@ Parameters:
@   r0: source address (compressed data)
@   r1: destination address (decompressing)
@ Returns:
@   r0: address after the last compressed byte consumed
@   r1: address after the last byte written
@ Clobbers r2, r3 (AAPCS caller-saved); r4-r6 are preserved.
@
@ Register usage:
@   r2: bit buffer, left-aligned, with a 1 sentinel below the last unread bit
@   r4: length / Elias value
@   r5: last offset (positive distance back from dst)
@   r3, r6: scratch
@ -----------------------------------------------------------------------------

        .syntax unified
        .cpu    cortex-m0plus
        .thumb

@ Reads one bit into the carry flag, reloading the buffer from (r0) when empty.
        .macro  GETBIT
        lsls    r2, r2, #1
        bne     1f
        ldrb    r2, [r0]
        adds    r0, r0, #1
        lsls    r2, r2, #1              @ (byte << 1) | sentinel, then
        adds    r2, r2, #1
        lsls    r2, r2, #24             @ carry = top bit of byte
1:
        .endm

        .text
        .align  2
        .global dzx0_turbo
        .thumb_func
        .type   dzx0_turbo, %function
dzx0_turbo:
        push    {r4-r6, lr}
        movs    r5, #1                  @ default offset 1
        movs    r2, #1
        lsls    r2, r2, #31             @ empty bit buffer
dzx0t_literals:
        bl      dzx0t_elias             @ obtain length
dzx0t_literals_copy:
        ldrb    r3, [r0]                @ copy literals
        adds    r0, r0, #1
        strb    r3, [r1]
        adds    r1, r1, #1
        subs    r4, r4, #1
        bne     dzx0t_literals_copy
        GETBIT                          @ copy from last offset or new offset?
        bcs     dzx0t_new_offset
        bl      dzx0t_elias             @ obtain length
dzx0t_copy:
        subs    r3, r1, r5              @ calculate destination - offset
dzx0t_copy_loop:
        ldrb    r6, [r3]                @ copy from offset
        adds    r3, r3, #1
        strb    r6, [r1]
        adds    r1, r1, #1
        subs    r4, r4, #1
        bne     dzx0t_copy_loop
        GETBIT                          @ copy from literals or new offset?
        bcc     dzx0t_literals
dzx0t_new_offset:
        bl      dzx0t_elias_inv         @ obtain offset MSB
        movs    r3, #1
        lsls    r3, r3, #8
        cmp     r4, r3
        beq     dzx0t_done              @ check end marker
        ldrb    r3, [r0]                @ obtain offset LSB
        adds    r0, r0, #1
        lsls    r4, r4, #7
        lsrs    r5, r3, #1
        subs    r5, r4, r5              @ preserve new offset
        lsrs    r3, r3, #1              @ last offset bit becomes first length bit
        bcs     dzx0t_length_two
        bl      dzx0t_elias_data0       @ obtain length
        adds    r4, r4, #1
        b       dzx0t_copy
dzx0t_length_two:
        movs    r4, #2
        b       dzx0t_copy
dzx0t_done:
        pop     {r4-r6, pc}

@ Interlaced Elias gamma coding, result in r4.
        .thumb_func
dzx0t_elias:
        movs    r4, #1
dzx0t_elias_ctl:
        GETBIT
        bcs     dzx0t_elias_ret
dzx0t_elias_data:
        GETBIT
        adcs    r4, r4, r4
        b       dzx0t_elias_ctl
dzx0t_elias_ret:
        bx      lr

@ Same, but the first control bit (0) has already been consumed.
        .thumb_func
dzx0t_elias_data0:
        movs    r4, #1
        b       dzx0t_elias_data

@ Interlaced Elias gamma coding with inverted data bits (offset MSB), result in r4.
        .thumb_func
dzx0t_elias_inv:
        movs    r4, #1
dzx0t_elias_inv_ctl:
        GETBIT
        bcs     dzx0t_elias_inv_ret
        GETBIT
        sbcs    r6, r6, r6              @ r6 = carry - 1
        lsls    r4, r4, #1
        subs    r4, r4, r6              @ append inverted bit
        b       dzx0t_elias_inv_ctl
dzx0t_elias_inv_ret:
        bx      lr

        .size   dzx0_turbo, . - dzx0_turbo
@ -----------------------------------------------------------------------------
