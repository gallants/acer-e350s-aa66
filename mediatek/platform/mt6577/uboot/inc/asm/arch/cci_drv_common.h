#ifndef _CCI_DRV_COMMON_H
#define _CCI_DRV_COMMON_H

extern bool cci_platform_is_evt(void);
extern bool cci_platform_is_dvt1(void);
extern bool cci_platform_is_dvt2(void);
extern bool cci_platform_is_pvt(void);
extern bool cci_platform_is_mp(void);

extern void bmt_charger_ov_check(void);
extern kal_bool pmic_chrdet_status(void);

#endif /* _CCI_DRV_COMMON_H */
