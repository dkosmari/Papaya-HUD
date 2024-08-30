/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WPAD_STATUS_H
#define WPAD_STATUS_H

#include <wut.h>
#include <padscore/wpad.h>

#ifdef __cplusplus
extern "C" {
#endif


#define WPAD_MAX_IR_DOTS 4


typedef struct WPADbVec2D
{
    int8_t x;
    int8_t y;
} WPADbVec2D;
WUT_CHECK_OFFSET(WPADbVec2D, 0x00, x);
WUT_CHECK_OFFSET(WPADbVec2D, 0x01, y);
WUT_CHECK_SIZE(WPADbVec2D, 0x02);


typedef struct WPADVec3D
{
    int16_t x;
    int16_t y;
    int16_t z;
} WPADVec3D;
WUT_CHECK_OFFSET(WPADVec3D, 0x00, x);
WUT_CHECK_OFFSET(WPADVec3D, 0x02, y);
WUT_CHECK_OFFSET(WPADVec3D, 0x04, z);
WUT_CHECK_SIZE(WPADVec3D, 0x06);


typedef struct WPADIRDot
{
    WPADVec2D pos;
    uint16_t  size;
    uint8_t   id;
} WPADIRDot;
WUT_CHECK_OFFSET(WPADIRDot, 0x00, pos);
WUT_CHECK_OFFSET(WPADIRDot, 0x04, size);
WUT_CHECK_OFFSET(WPADIRDot, 0x06, id);
WUT_CHECK_SIZE(WPADIRDot, 0x08);


typedef struct WPADStatus
{
    uint16_t  buttons;
    WPADVec3D acc;
    WPADIRDot ir[WPAD_MAX_IR_DOTS];
    uint8_t   extensionType;
    int8_t    error;
} WPADStatus;
WUT_CHECK_OFFSET(WPADStatus, 0x00, buttons);
WUT_CHECK_OFFSET(WPADStatus, 0x02, acc);
WUT_CHECK_OFFSET(WPADStatus, 0x08, ir);
WUT_CHECK_OFFSET(WPADStatus, 0x28, extensionType);
WUT_CHECK_OFFSET(WPADStatus, 0x29, error);
WUT_CHECK_SIZE(WPADStatus, 0x2a);


typedef struct WPADNunchukStatus
{
    WPADStatus core;
    WPADVec3D  acc;
    WPADbVec2D stick;
} WPADNunchukStatus;
WUT_CHECK_OFFSET(WPADNunchukStatus, 0x00, core);
WUT_CHECK_OFFSET(WPADNunchukStatus, 0x2a, acc);
WUT_CHECK_OFFSET(WPADNunchukStatus, 0x30, stick);
WUT_CHECK_SIZE(WPADNunchukStatus, 0x32);


typedef struct WPADClassicStatus
{
    WPADStatus core;
    uint16_t   buttons;
    WPADVec2D  leftStick;
    WPADVec2D  rightStick;
    uint8_t    leftTrigger;
    uint8_t    rightTrigger;
} WPADClassicStatus;
WUT_CHECK_OFFSET(WPADClassicStatus, 0x00, core);
WUT_CHECK_OFFSET(WPADClassicStatus, 0x2a, buttons);
WUT_CHECK_OFFSET(WPADClassicStatus, 0x2c, leftStick);
WUT_CHECK_OFFSET(WPADClassicStatus, 0x30, rightStick);
WUT_CHECK_OFFSET(WPADClassicStatus, 0x34, leftTrigger);
WUT_CHECK_OFFSET(WPADClassicStatus, 0x35, rightTrigger);
WUT_CHECK_SIZE(WPADClassicStatus, 0x36);


typedef struct WPADProStatus
{
    WPADStatus core;
    uint32_t   buttons;
    WPADVec2D  leftStick;
    WPADVec2D  rigtStick;
    BOOL       charging;
    BOOL       wired;
} WPADProStatus;
WUT_CHECK_OFFSET(WPADProStatus, 0x00, core);
WUT_CHECK_OFFSET(WPADProStatus, 0x2c, buttons);
WUT_CHECK_OFFSET(WPADProStatus, 0x30, leftStick);
WUT_CHECK_OFFSET(WPADProStatus, 0x34, rigtStick);
WUT_CHECK_OFFSET(WPADProStatus, 0x38, charging);
WUT_CHECK_OFFSET(WPADProStatus, 0x3c, wired);
WUT_CHECK_SIZE(WPADProStatus, 0x40);


// TODO: add structs for train, balance board controllers


#ifdef __cplusplus
}
#endif

#endif
