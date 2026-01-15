/* goldenmate-hid.c - subdriver to monitor Goldenmate USB/HID devices with NUT
 *
 *  Copyright (C)
 *  2003 - 2012 Arnaud Quette <ArnaudQuette@Eaton.com>
 *  2005 - 2006 Peter Selinger <selinger@users.sourceforge.net>
 *  2008 - 2009 Arjen de Korte <adkorte-guest@alioth.debian.org>
 *  2013 Charles Lepple <clepple+nut@gmail.com>
 *
 *  TODO: Add year and name for new subdriver author (contributor)
 *  Mention in docs/acknowledgements.txt if this is a vendor contribution
 *
 *  Note: this subdriver was initially generated as a "stub" by the
 *  gen-usbhid-subdriver script. It must be customized.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "usbhid-ups.h"
#include "goldenmate-hid.h"
#include "main.h"       /* for getval() */
#include "usb-common.h"

#define GOLDENMATE_HID_VERSION  "Goldenmate HID 0.1"
/* FIXME: experimental flag to be put in upsdrv_info */

/* Goldenmate */
#define GOLDENMATE_VENDORID     0x075d

/* USB IDs device table */
static usb_device_id_t goldenmate_usb_device_table[] = {
        /* Goldenmate */
        { USB_DEVICE(GOLDENMATE_VENDORID, 0x0300), NULL },

        /* Terminating entry */
        { 0, 0, NULL }
};


/* --------------------------------------------------------------- */
/*      Vendor-specific usage table */
/* --------------------------------------------------------------- */

/* GOLDENMATE usage table */
static usage_lkp_t goldenmate_usage_lkp[] = {
        { NULL, 0 }
};

static usage_tables_t goldenmate_utab[] = {
        goldenmate_usage_lkp,
        hid_usage_lkp,
        NULL,
};

/* --------------------------------------------------------------- */
/* HID2NUT lookup table                                            */
/* --------------------------------------------------------------- */

static hid_info_t goldenmate_hid2nut[] = {

/* Please revise values discovered by data walk for mappings to
 * docs/nut-names.txt and group the rest under the ifdef below:
 */

#if WITH_UNMAPPED_DATA_POINTS
        /* Capacity Mode appears to be for defining the capacity values (Remaining, Full, Design, etc.) - 0: maH, (used in SMB), 1: mwH (used in SMB), 2: % */
        { "unmapped.ups.powersummary.capacitymode", 0, 0, "UPS.PowerSummary.CapacityMode", NULL, "%.0f", 0, NULL },
        { "unmapped.ups.powersummary.batterypresent", 0, 0, "UPS.PowerSummary.BatteryPresent", NULL, "%.0f", 0, NULL },
        { "unmapped.ups.powersummary.flowid", 0, 0, "UPS.PowerSummary.FlowID", NULL, "%.0f", 0, NULL },
        { "unmapped.ups.powersummary.powersummaryid", 0, 0, "UPS.PowerSummary.PowerSummaryID", NULL, "%.0f", 0, NULL },
        { "unmapped.ups.powersummary.presentstatus.atratetimetoempty", 0, 0, "UPS.PowerSummary.PresentStatus.AtRateTimeToEmpty", NULL, "%.0f", 0, NULL },
        { "unmapped.ups.powersummary.presentstatus.averagetimetoempty", 0, 0, "UPS.PowerSummary.PresentStatus.AverageTimeToEmpty", NULL, "%.0f", 0, NULL },
        { "unmapped.ups.powersummary.rechargeable", 0, 0, "UPS.PowerSummary.Rechargeable", NULL, "%.0f", HU_FLAG_STATIC, NULL }, /* Read only */
#endif  /* if WITH_UNMAPPED_DATA_POINTS */

        /* Device Information */
        { "device.mfr", 0, 0, "UPS.PowerSummary.iManufacturer", NULL, "%s", HU_FLAG_STATIC, stringid_conversion }, /* Read only */
        { "device.model", 0, 0, "UPS.PowerSummary.iProduct", NULL, "%s", HU_FLAG_STATIC, stringid_conversion }, /* Read only */
        { "device.serial", 0, 0, "UPS.PowerSummary.iSerialNumber", NULL, "%s", HU_FLAG_STATIC, stringid_conversion }, /* Read only */

        /* Battery & Power Data */
        { "battery.charge", 0, 0, "UPS.PowerSummary.RemainingCapacity", NULL, "%.0f", 0, NULL },
        { "battery.charge.low", 0, 0, "UPS.PowerSummary.WarningCapacityLimit", NULL, "%.0f", HU_FLAG_STATIC, NULL }, /* Read only */
        { "battery.runtime", 0, 0, "UPS.PowerSummary.RunTimeToEmpty", NULL, "%.0f", 0, NULL },
        { "battery.voltage", 0, 0, "UPS.PowerSummary.Voltage", NULL, "%.1f", 0, NULL },
        { "battery.voltage.nominal", 0, 0, "UPS.PowerSummary.ConfigVoltage", NULL, "%.1f", 0, NULL },
        { "battery.type", 0, 0, "UPS.PowerSummary.iDeviceChemistry", NULL, "%s", HU_FLAG_STATIC, stringid_conversion }, /* Read only */

        /* Capacity & Design */
        { "battery.capacity.design", 0, 0, "UPS.PowerSummary.DesignCapacity", NULL, "%.0f", HU_FLAG_STATIC, NULL }, /* Read only */
        { "battery.capacity.full", 0, 0, "UPS.PowerSummary.FullChargeCapacity", NULL, "%.0f", 0, NULL },

        /* UPS Status (The bits inside PresentStatus Logical Collection) */
        { "BOOL", 0, 0, "UPS.PowerSummary.PresentStatus.ACPresent", NULL, NULL, HU_FLAG_QUICK_POLL, online_info },
        { "BOOL", 0, 0, "UPS.PowerSummary.PresentStatus.Charging", NULL, NULL, HU_FLAG_QUICK_POLL, charging_info },
        { "BOOL", 0, 0, "UPS.PowerSummary.PresentStatus.Discharging", NULL, NULL, HU_FLAG_QUICK_POLL, discharging_info },
        { "BOOL", 0, 0, "UPS.PowerSummary.PresentStatus.BelowRemainingCapacityLimit", NULL, NULL, HU_FLAG_QUICK_POLL, lowbatt_info },
        { "BOOL", 0, 0, "UPS.PowerSummary.PresentStatus.BatteryPresent", NULL, NULL, 0, nobattery_info },

        /* Shutdown / Control */
        { "ups.delay.start", ST_FLAG_RW | ST_FLAG_STRING, 10, "UPS.PowerSummary.DelayBeforeStartup", NULL, "%.0f", 0, NULL },
        { "ups.delay.shutdown", ST_FLAG_RW | ST_FLAG_STRING, 10, "UPS.PowerSummary.DelayBeforeShutdown", NULL, "%.0f", 0, NULL },
        { "ups.timer.start", 0, 0, "UPS.PowerSummary.DelayBeforeStartup", NULL, "%.0f", HU_FLAG_QUICK_POLL, NULL },
        { "ups.timer.shutdown", 0, 0, "UPS.PowerSummary.DelayBeforeShutdown", NULL, "%.0f", HU_FLAG_QUICK_POLL, NULL },

       	/* instant commands */
        { "load.on.delay", 0, 0, "UPS.PowerSummary.DelayBeforeStartup", NULL, "%.0f", HU_TYPE_CMD, NULL },
        { "load.off.delay", 0, 0, "UPS.PowerSummary.DelayBeforeShutdown", NULL, "%.0f", HU_TYPE_CMD, NULL },

        /* end of structure. */
        { NULL, 0, 0, NULL, NULL, NULL, 0, NULL }
};

static const char *goldenmate_format_model(HIDDevice_t *hd) {
        return hd->Product;
}

static const char *goldenmate_format_mfr(HIDDevice_t *hd) {
        return hd->Vendor ? hd->Vendor : "Goldenmate";
}

static const char *goldenmate_format_serial(HIDDevice_t *hd) {
        return hd->Serial;
}

/* this function allows the subdriver to "claim" a device: return 1 if
 * the device is supported by this subdriver, else 0. */
static int goldenmate_claim(HIDDevice_t *hd)
{
        int status = is_usb_device_supported(goldenmate_usb_device_table, hd);

        switch (status)
        {
        case POSSIBLY_SUPPORTED:
                /* by default, reject, unless the productid option is given */
                if (getval("productid")) {
                        return 1;
                }
                possibly_supported("Goldenmate", hd);
                return 0;

        case SUPPORTED:
                return 1;

        case NOT_SUPPORTED:
        default:
                return 0;
        }
}

subdriver_t goldenmate_subdriver = {
        GOLDENMATE_HID_VERSION,
        goldenmate_claim,
        goldenmate_utab,
        goldenmate_hid2nut,
        goldenmate_format_model,
        goldenmate_format_mfr,
        goldenmate_format_serial,
        fix_report_desc,        /* may optionally be customized, see cps-hid.c for example */
};