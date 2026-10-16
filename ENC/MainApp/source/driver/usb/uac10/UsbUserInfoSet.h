#ifndef __USBUSERINFOSET_H_
#define __USBUSERINFOSET_H_

uint32_t UsbUserInfoSelectAlgGet(void);
uint32_t UsbUserInfoSelectCtrlGet(void);
uint32_t UsbUserInfoSelectFsTypeGet(void);
uint32_t UsbUserInfoSelectSidetoneGet(void);

void UsbUserInfoInit(void);

#endif /* End of #ifndef __USBUSERINFOSET_H_ */
