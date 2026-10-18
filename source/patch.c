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

    // 3. AbstractMenu::GetBounds() escala X por OS_SCREEN_H/movieH e Y por OS_SCREEN_W/movieW
    // (ejes cruzados; invisible a 800x480, a 960x544 descentra el auto del icono Garage).
    // Intercambiar los literales GOT de OS_SCREEN_H (0x141c @0x119584) y OS_SCREEN_W (0xe98 @0x11958c).
    uint32_t got_screen_w = 0x00000e98, got_screen_h = 0x0000141c;
    kuKernelCpuUnrestrictedMemcpy((void *)(so_mod.text_base + 0x119584), &got_screen_w, sizeof(got_screen_w));
    kuKernelCpuUnrestrictedMemcpy((void *)(so_mod.text_base + 0x11958c), &got_screen_h, sizeof(got_screen_h));
    // El extent del movie tambien esta cruzado: X divide por (frame.y_max-y_min) [0xc0/0xbc] e Y por
    // (frame.x_max-x_min) [0xb8/0xb4]. Solo con el swap de arriba X quedaba x2.0 e Y x0.68 (minimapa,
    // banderas y auto del Garage desplazados). Intercambiar los offsets en los `movs r3,#imm`.
    static const struct { uint32_t addr; uint8_t imm; } bounds_ext[] = {
        { 0x1194fa, 0xb8 }, { 0x119500, 0xb4 },  // X: x_max - x_min (era 0xc0/0xbc)
        { 0x119540, 0xc0 }, { 0x119546, 0xbc },  // Y: y_max - y_min (era 0xb8/0xb4)
    };
    for (unsigned i = 0; i < sizeof(bounds_ext) / sizeof(bounds_ext[0]); i++)
        kuKernelCpuUnrestrictedMemcpy((void *)(so_mod.text_base + bounds_ext[i].addr), &bounds_ext[i].imm, 1);

    // 4. Rivales en LOD 3/4 (7000-15000 de distancia) se ven rojos con la textura por defecto:
    // esos meshes no reciben la pintura de ReplaceTexture y el cambio a LOD 2 se nota.
    // s_iPhone3GS_conditions_LOD (rodata 0x2c42dc, {lod,dist,trackDist,sectionDiff,refresh}):
    // el campo lod de las entradas 3 y 4 pasa a 2. DeviceConfig::SetupGameForDevice copia
    // esta tabla a s_LODConditions durante nativeInit, despues de so_patch().
    uint32_t lod2 = 2;
    kuKernelCpuUnrestrictedMemcpy((void *)(so_mod.text_base + 0x2c4304), &lod2, sizeof(lod2));
    kuKernelCpuUnrestrictedMemcpy((void *)(so_mod.text_base + 0x2c4318), &lod2, sizeof(lod2));
}
