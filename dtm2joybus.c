#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include "format.h"
#include <string.h>

void printstate(int idx, struct status *status){
	printf("idx %05d: a=%d, start=%d, l=%d, r=%d, stick=0x%x,0x%x\n",
			idx, status->buttons.a,
			status->buttons.start,
			status->buttons.l,
			status->buttons.r,
			status->stick.x,
			status->stick.y);
}


/* Translate from DTM to JoyBus responses */
struct status
translate(int idx, struct ControllerState *src, struct status *dest){
	struct status status = {};
	//struct buttons buttons;

	status.buttons.use_origin = 1;

	if (src->A)
		status.buttons.a = 1;
	if (src->B)
		status.buttons.b = 1;
	if (src->X)
		status.buttons.x = 1;
	if (src->Y)
		status.buttons.y = 1;
	if (src->Z)
		status.buttons.z = 1;
	if (src->Start)
		status.buttons.start = 1;
	if (src->DPadUp)
		status.buttons.up = 1;
	if (src->DPadDown)
		status.buttons.down = 1;
	if (src->DPadLeft)
		status.buttons.left = 1;
	if (src->DPadRight)
		status.buttons.right = 1;
	if (src->L)
		status.buttons.l = 1;
	if (src->R)
		status.buttons.r = 1;


	//status.buttons = buttons;
	status.stick.x = src->AnalogStickX;
	status.stick.y = src->AnalogStickY;

	status.mode0.substick.x = src->CStickX;
	status.mode0.substick.y = src->CStickY;
	status.mode0.trigger.l = src->TriggerL;
	status.mode0.trigger.r = src->TriggerR;

	memcpy(dest, &status, sizeof(struct status));
	return status;

}

void print_header(struct DTMHeader *header){
	printf("controllers: 0x%02X\n", header->controllers);
	printf("lagCount: %lld\n", header->lagCount);
	printf("frameCount: %lld\n", header->frameCount);
	printf("inputCount: %lld\n", header->inputCount);
	printf("bFromSaveState: %d\n", header->bFromSaveState);
	printf("numRerecords: %d\n", header->numRerecords);
	printf("recordingStartTime: %lld\n", header->recordingStartTime);
	printf("saveconfig: %d\n", header->bSaveConfig);
	printf("skipidle: %d\n", header->bSkipIdle);
	printf("dualcore: %d\n", header->bDualCore);
	printf("progressive: %d\n", header->bProgressive);
	printf("dsphle: %d\n", header->bDSPHLE);
	printf("fastdiskspeed: %d\n", header->bFastDiscSpeed);
	printf("efbaccessenable: %d\n", header->bEFBAccessEnable);
	printf("efbcopyenable: %d\n", header->bEFBCopyEnable);
	printf("skipefbcopytoram: %d\n", header->bSkipEFBCopyToRam);
	printf("efbcopycacheenable: %d\n", header->bEFBCopyCacheEnable);
	printf("efbemulateformatchanges: %d\n", header->bEFBEmulateFormatChanges);
	printf("bImmediateXFB: %d\n", header->bImmediateXFB);
	printf("SkipXFBCopytoRam: %d\n", header->bSkipXFBCopyToRam);
	printf("memcards: %02x\n", header->memcards);
	printf("clearsave: %d\n", header->bClearSave);
	printf("syncgpu: %d\n", header->bSyncGPU);
	printf("netplay: %d\n", header->bNetPlay);
	printf("PAL60: %d\n", header->bPAL60);
}


int main(int argc, char *argv[]){
	size_t size;
	int total;
	int padding, cut;
	FILE *dtm_file;

	struct status pad = {};

	// Can probably just use this as a mock "empty" response for padding
	uint8_t empty[8] = { 0x00,0x80,0x76,0x7a,0x7e,0x85,0x2c,0x00 };
	memcpy(&pad, &empty, sizeof(struct status));

	// Handle some cmdline args
	if (argc < 4) {
		printf("./dtm2joybus <input .DTM> <padding inputs> <cut inputs>\n");
		printf("Writes to /tmp/inputs.bin, for now.\n");
		exit(-1);
	}
	printf("%s %s\n", argv[0], argv[1]);

	// Get number of padding inputs
	padding = atoi(argv[2]);
	cut = atoi(argv[3]);
	printf("Using %d padding inputs\n", padding);

	// Open a DTM and read into buffer
	dtm_file = fopen(argv[1], "rb");
	if (!dtm_file) {
		printf("File not found\n");
		exit(-1);
	}
	fseek(dtm_file, 0L, SEEK_END);
	size = ftell(dtm_file);
	fseek(dtm_file, 0L, SEEK_SET);
	printf("File is 0x%x bytes\n", size);
	unsigned char buf[size + 0x10000];
	fread(buf, size, 1, dtm_file);
	fclose(dtm_file);

	// Feedback about the DTM header
	struct DTMHeader *header = (struct DTMHeader*)buf;
	print_header(header);

	// Read in DTM inputs into an array
	struct ControllerState *cs = (struct ControllerState*)&buf[0x100];
	int dtm_inputs = header->inputCount;
	struct ControllerState dtm_array[dtm_inputs];
	for (int z = 0; z < dtm_inputs; z++){
		memcpy(&dtm_array[z], cs, sizeof(struct ControllerState));
		cs++;
	}
	
	// Convert DTM inputs to Joybus responses
	struct status joybus_array[dtm_inputs];
	for(int i = 0; i < dtm_inputs; i++){
		translate(i, &dtm_array[i], &joybus_array[i]);
		printstate(i, &joybus_array[i]);
		cs++;
	}

	// Build a final array
	total = dtm_inputs + padding - cut;
	printf("Total inputs will be %d\n", total);
	struct status output[total];
	int cursor = 0;
	for(int i = 0; i < padding; i++){
		memcpy(&output[cursor], &pad, sizeof(struct status));
		cursor++;
	}
	for(int i = cut; i < dtm_inputs; i++){
		memcpy(&output[cursor], &joybus_array[i], sizeof(struct status));
		cursor++;
	}
	printf("Copied %d inputs\n", cursor);

	// Write binary Joybus responses to a file
	FILE *output_file = fopen("/tmp/inputs.bin", "wb");
	if (!output_file) {
		printf("Couldn't open file for writing\n");
		exit(-1);
	}
	fwrite(output, sizeof(struct status), total, output_file);
	printf("Wrote output\n");
	fclose(output_file);
}
