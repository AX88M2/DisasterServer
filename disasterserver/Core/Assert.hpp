#pragma once

#define AssertOrDisconnect(client, condition) \
    if(!(condition)) { \
        client.disconnect(DisconnectReason::OTHER, "AssertOrDisconnect({}) failed!", #condition); return false; \
    }


#define RAssert(x) if (!(x)) { Error("RAssert({}) failed!", #x); return false; }
#define RAssertEx(x) if (!(x)) { Error("RAssert({}) failed!", #x); }