
#include "psbt.h"
#include <stdio.h>
#include <assert.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// unsigned transaction from the bip174 test vector section
// 02000000022e8c7d8d37c427e060ec002ec1c2bc30196fc2f75d6a8844cbc03651c081430a0100000000ffffffff96a04e0cc636f377933e3d93accc627faacdbcdb5a9624df1b490bd045f24d2c0000000000ffffffff01e02be50e0000000017a914b53bb0dc1db8c8d803e3e39f784d42e4737ffa0d8700000000
static const unsigned char transaction[] = {
  0x02, 0x00, 0x00, 0x00, 0x02, 0x2e, 0x8c, 0x7d, 0x8d, 0x37, 0xc4, 0x27, 0xe0, 0x60, 0xec, 0x00, 0x2e, 0xc1, 0xc2, 0xbc, 0x30, 0x19, 0x6f, 0xc2, 0xf7, 0x5d, 0x6a, 0x88, 0x44, 0xcb, 0xc0, 0x36, 0x51, 0xc0, 0x81, 0x43, 0x0a, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x96, 0xa0, 0x4e, 0x0c, 0xc6, 0x36, 0xf3, 0x77, 0x93, 0x3e, 0x3d, 0x93, 0xac, 0xcc, 0x62, 0x7f, 0xaa, 0xcd, 0xbc, 0xdb, 0x5a, 0x96, 0x24, 0xdf, 0x1b, 0x49, 0x0b, 0xd0, 0x45, 0xf2, 0x4d, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x01, 0xe0, 0x2b, 0xe5, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x17, 0xa9, 0x14, 0xb5, 0x3b, 0xb0, 0xdc, 0x1d, 0xb8, 0xc8, 0xd8, 0x03, 0xe3, 0xe3, 0x9f, 0x78, 0x4d, 0x42, 0xe4, 0x73, 0x7f, 0xfa, 0x0d, 0x87, 0x00, 0x00, 0x00, 0x00
};

// 522103c8727ce35f1c93eb0be21406ee9a923c89219fe9c9e8504c8314a6a22d1295c02103c74dc710c407d7db6e041ee212d985cd2826d93f806ed44912b9a1da691c977352ae
static const unsigned char redeem_script_a[] = {0x52, 0x21, 0x03, 0xc8, 0x72, 0x7c, 0xe3, 0x5f, 0x1c, 0x93, 0xeb, 0x0b, 0xe2, 0x14, 0x06, 0xee, 0x9a, 0x92, 0x3c, 0x89, 0x21, 0x9f, 0xe9, 0xc9, 0xe8, 0x50, 0x4c, 0x83, 0x14, 0xa6, 0xa2, 0x2d, 0x12, 0x95, 0xc0, 0x21, 0x03, 0xc7, 0x4d, 0xc7, 0x10, 0xc4, 0x07, 0xd7, 0xdb, 0x6e, 0x04, 0x1e, 0xe2, 0x12, 0xd9, 0x85, 0xcd, 0x28, 0x26, 0xd9, 0x3f, 0x80, 0x6e, 0xd4, 0x49, 0x12, 0xb9, 0xa1, 0xda, 0x69, 0x1c, 0x97, 0x73, 0x52, 0xae};

// 0020a8f44467bf171d51499153e01c0bd6291109fc38bd21b3c3224c9dc6b57590df
static const unsigned char redeem_script_b[] = {0x00, 0x20, 0xa8, 0xf4, 0x44, 0x67, 0xbf, 0x17, 0x1d, 0x51, 0x49, 0x91,  0x53, 0xe0, 0x1c, 0x0b, 0xd6, 0x29, 0x11, 0x09, 0xfc, 0x38, 0xbd, 0x21,  0xb3, 0xc3, 0x22, 0x4c, 0x9d, 0xc6, 0xb5, 0x75, 0x90, 0xdf};

// expected
// 70736274ff01007c02000000022e8c7d8d37c427e060ec002ec1c2bc30196fc2f75d6a8844cbc03651c081430a0100000000ffffffff96a04e0cc636f377933e3d93accc627faacdbcdb5a9624df1b490bd045f24d2c0000000000ffffffff01e02be50e0000000017a914b53bb0dc1db8c8d803e3e39f784d42e4737ffa0d87000000001501203736c3c06053896d7041ce8f5bae3df76cc49147522103c8727ce35f1c93eb0be21406ee9a923c89219fe9c9e8504c8314a6a22d1295c02103c74dc710c407d7db6e041ee212d985cd2826d93f806ed44912b9a1da691c977352ae1501f3ba8a120d960ae07d1dbe6f0c37fb4c926d76d5220020a8f44467bf171d51499153e01c0bd6291109fc38bd21b3c3224c9dc6b57590df2102a8f44467bf171d51499153e01c0bd6291109fc38bd21b3c3224c9dc6b57590df47522102e80dec31d167865c1685e9d7a9291e66a4ea22c65cfee324289a1667ccda3b87210258cbbc3cb295a8bebac233aadc7773978804993798be5390ab444f6dd4c5327e52ae000100fdff0002000000018b2dd2f735d0a9338af96402a8a91e4841cd3fed882362e7329fb04f1ff65325000000006a473044022077bedfea9910c9ba4e00dec941dace974f8b47349992c5d4312c1cf5796cce5502206164e6bfff7ac11590064ca571583709337c8a38973db2e70f4e9d93b3bcce1d0121032d64447459784e37cb2dda366c697adbbdc8aae2ad6db74ed2dade39d75882fafeffffff0382b42a04000000001976a914da533648fd339d5797790e6bb1667d9e86fdfb6888ac80f0fa020000000017a914203736c3c06053896d7041ce8f5bae3df76cc4918700b4c4040000000017a914b53bb0dc1db8c8d803e3e39f784d42e4737ffa0d879e2f13000001012000c2eb0b0000000017a914f3ba8a120d960ae07d1dbe6f0c37fb4c926d76d58700
static const unsigned char expected_psbt[] = {
  0x70, 0x73, 0x62, 0x74, 0xff, 0x01, 0x00, 0x7c, 0x02, 0x00, 0x00, 0x00, 0x02, 0x2e, 0x8c, 0x7d, 0x8d, 0x37, 0xc4, 0x27, 0xe0, 0x60, 0xec, 0x00, 0x2e, 0xc1, 0xc2, 0xbc, 0x30, 0x19, 0x6f, 0xc2, 0xf7, 0x5d, 0x6a, 0x88, 0x44, 0xcb, 0xc0, 0x36, 0x51, 0xc0, 0x81, 0x43, 0x0a, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x96, 0xa0, 0x4e, 0x0c, 0xc6, 0x36, 0xf3, 0x77, 0x93, 0x3e, 0x3d, 0x93, 0xac, 0xcc, 0x62, 0x7f, 0xaa, 0xcd, 0xbc, 0xdb, 0x5a, 0x96, 0x24, 0xdf, 0x1b, 0x49, 0x0b, 0xd0, 0x45, 0xf2, 0x4d, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x01, 0xe0, 0x2b, 0xe5, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x17, 0xa9, 0x14, 0xb5, 0x3b, 0xb0, 0xdc, 0x1d, 0xb8, 0xc8, 0xd8, 0x03, 0xe3, 0xe3, 0x9f, 0x78, 0x4d, 0x42, 0xe4, 0x73, 0x7f, 0xfa, 0x0d, 0x87, 0x00, 0x00, 0x00, 0x00, 0x15, 0x01, 0x20, 0x37, 0x36, 0xc3, 0xc0, 0x60, 0x53, 0x89, 0x6d, 0x70, 0x41, 0xce, 0x8f, 0x5b, 0xae, 0x3d, 0xf7, 0x6c, 0xc4, 0x91, 0x47, 0x52, 0x21, 0x03, 0xc8, 0x72, 0x7c, 0xe3, 0x5f, 0x1c, 0x93, 0xeb, 0x0b, 0xe2, 0x14, 0x06, 0xee, 0x9a, 0x92, 0x3c, 0x89, 0x21, 0x9f, 0xe9, 0xc9, 0xe8, 0x50, 0x4c, 0x83, 0x14, 0xa6, 0xa2, 0x2d, 0x12, 0x95, 0xc0, 0x21, 0x03, 0xc7, 0x4d, 0xc7, 0x10, 0xc4, 0x07, 0xd7, 0xdb, 0x6e, 0x04, 0x1e, 0xe2, 0x12, 0xd9, 0x85, 0xcd, 0x28, 0x26, 0xd9, 0x3f, 0x80, 0x6e, 0xd4, 0x49, 0x12, 0xb9, 0xa1, 0xda, 0x69, 0x1c, 0x97, 0x73, 0x52, 0xae, 0x15, 0x01, 0xf3, 0xba, 0x8a, 0x12, 0x0d, 0x96, 0x0a, 0xe0, 0x7d, 0x1d, 0xbe, 0x6f, 0x0c, 0x37, 0xfb, 0x4c, 0x92, 0x6d, 0x76, 0xd5, 0x22, 0x00, 0x20, 0xa8, 0xf4, 0x44, 0x67, 0xbf, 0x17, 0x1d, 0x51, 0x49, 0x91, 0x53, 0xe0, 0x1c, 0x0b, 0xd6, 0x29, 0x11, 0x09, 0xfc, 0x38, 0xbd, 0x21, 0xb3, 0xc3, 0x22, 0x4c, 0x9d, 0xc6, 0xb5, 0x75, 0x90, 0xdf, 0x21, 0x02, 0xa8, 0xf4, 0x44, 0x67, 0xbf, 0x17, 0x1d, 0x51, 0x49, 0x91, 0x53, 0xe0, 0x1c, 0x0b, 0xd6, 0x29, 0x11, 0x09, 0xfc, 0x38, 0xbd, 0x21, 0xb3, 0xc3, 0x22, 0x4c, 0x9d, 0xc6, 0xb5, 0x75, 0x90, 0xdf, 0x47, 0x52, 0x21, 0x02, 0xe8, 0x0d, 0xec, 0x31, 0xd1, 0x67, 0x86, 0x5c, 0x16, 0x85, 0xe9, 0xd7, 0xa9, 0x29, 0x1e, 0x66, 0xa4, 0xea, 0x22, 0xc6, 0x5c, 0xfe, 0xe3, 0x24, 0x28, 0x9a, 0x16, 0x67, 0xcc, 0xda, 0x3b, 0x87, 0x21, 0x02, 0x58, 0xcb, 0xbc, 0x3c, 0xb2, 0x95, 0xa8, 0xbe, 0xba, 0xc2, 0x33, 0xaa, 0xdc, 0x77, 0x73, 0x97, 0x88, 0x04, 0x99, 0x37, 0x98, 0xbe, 0x53, 0x90, 0xab, 0x44, 0x4f, 0x6d, 0xd4, 0xc5, 0x32, 0x7e, 0x52, 0xae, 0x00, 0x01, 0x00, 0xfd, 0xff, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x8b, 0x2d, 0xd2, 0xf7, 0x35, 0xd0, 0xa9, 0x33, 0x8a, 0xf9, 0x64, 0x02, 0xa8, 0xa9, 0x1e, 0x48, 0x41, 0xcd, 0x3f, 0xed, 0x88, 0x23, 0x62, 0xe7, 0x32, 0x9f, 0xb0, 0x4f, 0x1f, 0xf6, 0x53, 0x25, 0x00, 0x00, 0x00, 0x00, 0x6a, 0x47, 0x30, 0x44, 0x02, 0x20, 0x77, 0xbe, 0xdf, 0xea, 0x99, 0x10, 0xc9, 0xba, 0x4e, 0x00, 0xde, 0xc9, 0x41, 0xda, 0xce, 0x97, 0x4f, 0x8b, 0x47, 0x34, 0x99, 0x92, 0xc5, 0xd4, 0x31, 0x2c, 0x1c, 0xf5, 0x79, 0x6c, 0xce, 0x55, 0x02, 0x20, 0x61, 0x64, 0xe6, 0xbf, 0xff, 0x7a, 0xc1, 0x15, 0x90, 0x06, 0x4c, 0xa5, 0x71, 0x58, 0x37, 0x09, 0x33, 0x7c, 0x8a, 0x38, 0x97, 0x3d, 0xb2, 0xe7, 0x0f, 0x4e, 0x9d, 0x93, 0xb3, 0xbc, 0xce, 0x1d, 0x01, 0x21, 0x03, 0x2d, 0x64, 0x44, 0x74, 0x59, 0x78, 0x4e, 0x37, 0xcb, 0x2d, 0xda, 0x36, 0x6c, 0x69, 0x7a, 0xdb, 0xbd, 0xc8, 0xaa, 0xe2, 0xad, 0x6d, 0xb7, 0x4e, 0xd2, 0xda, 0xde, 0x39, 0xd7, 0x58, 0x82, 0xfa, 0xfe, 0xff, 0xff, 0xff, 0x03, 0x82, 0xb4, 0x2a, 0x04, 0x00, 0x00, 0x00, 0x00, 0x19, 0x76, 0xa9, 0x14, 0xda, 0x53, 0x36, 0x48, 0xfd, 0x33, 0x9d, 0x57, 0x97, 0x79, 0x0e, 0x6b, 0xb1, 0x66, 0x7d, 0x9e, 0x86, 0xfd, 0xfb, 0x68, 0x88, 0xac, 0x80, 0xf0, 0xfa, 0x02, 0x00, 0x00, 0x00, 0x00, 0x17, 0xa9, 0x14, 0x20, 0x37, 0x36, 0xc3, 0xc0, 0x60, 0x53, 0x89, 0x6d, 0x70, 0x41, 0xce, 0x8f, 0x5b, 0xae, 0x3d, 0xf7, 0x6c, 0xc4, 0x91, 0x87, 0x00, 0xb4, 0xc4, 0x04, 0x00, 0x00, 0x00, 0x00, 0x17, 0xa9, 0x14, 0xb5, 0x3b, 0xb0, 0xdc, 0x1d, 0xb8, 0xc8, 0xd8, 0x03, 0xe3, 0xe3, 0x9f, 0x78, 0x4d, 0x42, 0xe4, 0x73, 0x7f, 0xfa, 0x0d, 0x87, 0x9e, 0x2f, 0x13, 0x00, 0x00, 0x01, 0x01, 0x20, 0x00, 0xc2, 0xeb, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x17, 0xa9, 0x14, 0xf3, 0xba, 0x8a, 0x12, 0x0d, 0x96, 0x0a, 0xe0, 0x7d, 0x1d, 0xbe, 0x6f, 0x0c, 0x37, 0xfb, 0x4c, 0x92, 0x6d, 0x76, 0xd5, 0x87, 0x00
};

int main(int argc, char *argv[])
{
	struct psbt psbt;
	char buffer[1024] = {0};
	enum psbt_result res;

	struct psbt_record rec;

	res = psbt_init(&psbt, buffer, 1024);
	assert(res == PSBT_OK);

	rec.type     = PSBT_TYPE_TRANSACTION;
	rec.key      = NULL;
	rec.key_size = 0;
	rec.val      = (char*)transaction;
	rec.val_size = ARRAY_SIZE(transaction);

	res = psbt_write_record(&psbt, &rec);
	assert(res == PSBT_OK);

	rec.type     = PSBT_TYPE_REDEEM_SCRIPT;
	rec.key      = "hash160ofredeemscript";
	rec.key_size = sizeof("hash160ofredeemscript");
	rec.val      = (unsigned char*)redeem_script_a;
	rec.val_size = ARRAY_SIZE(redeem_script_a);

	res = psbt_write_record(&psbt, &rec);
	assert(res == PSBT_OK);

	rec.type     = PSBT_TYPE_REDEEM_SCRIPT;
	rec.key      = "hash160ofredeemscript";
	rec.key_size = sizeof("hash160ofredeemscript");
	rec.val      = (unsigned char*)redeem_script_b;
	rec.val_size = ARRAY_SIZE(redeem_script_b);

	res = psbt_write_record(&psbt, &rec);
	assert(res == PSBT_OK);

	size_t size = psbt_size(&psbt);
	for (size_t i = 0; i < size; ++i)
		printf("%02x", psbt.data[i]);
	printf("\n");

	return 0;
}
