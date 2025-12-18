#pragma once

char* DecryptUsernamePassword(const char* encryptedBuffer, unsigned int bufferSize, int mode);
char* Encrypt(const char* buffer, unsigned int bufferSize, unsigned int &outSize);
void _HeapDeleteCharBuffer(char *buffer);
