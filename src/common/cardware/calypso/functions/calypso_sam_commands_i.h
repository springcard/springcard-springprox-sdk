#ifndef __CALYPSO_SAM_COMMANDS_I_H__
#define __CALYPSO_SAM_COMMANDS_I_H__

CALYPSO_RC CalypsoSamSetSW(CALYPSO_CTX_ST* ctx, CALYPSO_SZ recv_len);
CALYPSO_RC CalypsoSamGetResponse(CALYPSO_CTX_ST* ctx, CALYPSO_SZ* recv_len);

#endif
