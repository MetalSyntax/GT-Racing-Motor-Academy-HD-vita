/*
 * Copyright (C) 2023 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

/**
 * @file  patch.c
 * @brief Patching some of the .so internal functions or bridging them to native
 *        for better compatibility.
 */

#include <kubridge.h>
#include <so_util/so_util.h>

extern so_module so_mod;

void so_patch(void) {
    // 1. ControlsMenu::IsComboValid() en 0x00138f2c -> siempre retornar 1 (valido)
    // Permite guardar y usar cualquier esquema de control sin que el juego lo rechace o revierta
    // Thumb: movs r0, #1 (0x2001) ; bx lr (0x4770) -> little-endian 0x47702001
    uint32_t patch_combo_valid = 0x47702001;
    kuKernelCpuUnrestrictedMemcpy((void *)(so_mod.text_base + 0x138f2c), &patch_combo_valid, sizeof(patch_combo_valid));

    // 2. Control por defecto en CGameSettings::ResetControls():
    // 0xcdf62: str r2, [r0, #0x3c] (Throttle): movs r2, #1 (PedalsFar)
    uint8_t default_throttle = 0x01;
    kuKernelCpuUnrestrictedMemcpy((void *)(so_mod.text_base + 0xcdf62), &default_throttle, sizeof(default_throttle));

    // 0xcdf68: str r2, [r0, #0x40] (Steering): movs r2, #0 (0 = SideButtons / Botones fisicos en lugar de 2 = acelerometro)
    uint8_t default_steering = 0x00;
    kuKernelCpuUnrestrictedMemcpy((void *)(so_mod.text_base + 0xcdf68), &default_steering, sizeof(default_steering));
}
