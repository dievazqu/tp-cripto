
/**
 * Read and parse a wave file
 *
 * http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
 * 
 **/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wave.h"
#include "type.h"
#include "openssl-lib.h"
#define TRUE 1 
#define FALSE 0
#define MASK(i) (0x01<<i)
#define IS_BIT_ON(b,i) (b&MASK(i))
#define INT_TO_BYTE(i, index) ((i&(0xFF<<(24-index*8)))>>((24-index*8)))

// WAVE header structure


encryption_method enc_met;
block_method block_met;

BYTE buffer4[4];
BYTE buffer2[2];

FILE *ptr_wavefile;
FILE *out_wavefile;
char wavefilename[1024];
char outfilename[1024];
char infilename[1024];
BYTE *out_data;
struct HEADER header;
char extract;
int lsb_method;
int lsbe;
unsigned long out_data_size;
long i;
char *password;

void parsingError(){
	printf("Argumentos erroneos\n");
	exit(1);
}

void addLSB1(BYTE *in, long index, BYTE *buffer){
	int index_in_array = index/8;
	int index_in_byte = index%8;
	int shift = 7-index_in_byte;
	/*BYTE b = (in[index_in_array]>>shift)&0x01;
	*buffer&=0xFE;
	*buffer|=b;*/
	if(IS_BIT_ON(in[index_in_array],shift)){
		*buffer|=0x01;
	}else{
		*buffer&=0xFE;
	}
}

void addLSB4(BYTE *in, long index, BYTE *buffer){
	index/=4;
	int index_in_array = index/2;
	int index_in_byte = index%2;
	int shift = 4-index_in_byte*4;
	BYTE b = (in[index_in_array]>>shift)&0x0F;
	*buffer&=0xF0;
	*buffer|=b;
}

void getLSB1(BYTE *out, long index, BYTE data){
	int index_in_array = index/8;
	int index_in_byte = index%8;
	int  shift = 7-index_in_byte;
	out[index_in_array]|=(data&0x01)<<shift;
}

void getLSB2(BYTE *out, long index, BYTE data){
	int index_in_array = index/4;
	int index_in_byte = index%4;
	int shift = 6-index_in_byte*2;
	out[index_in_array]|=(data&0x03)<<shift;
}

void getLSB4(BYTE *out, long index, BYTE data){
	int index_in_array = index/2;
	int index_in_byte = index%2;
	int shift = 4-index_in_byte*4;
	out[index_in_array]|=(data&0x0f)<<shift;
}


int main(int argc, char **argv) {
	if(argc<7 || argc%2==1){
		parsingError();
	}
	lsbe=0;
	
	if(strcmp(argv[1],"-extract")==0){
		extract=1;
	}else if(strcmp(argv[1],"-embed")==0){
		extract=0;
	}else{
		parsingError();
	}
	int wavefilepresent=FALSE;
	int outfilepresent=FALSE;
	int infilepresent=FALSE;
	enc_met=AES128;
	block_met=CBC;
	lsb_method=-1;
	password=NULL;
	for(i=2; i<argc; i+=2){
		if(strcmp(argv[i], "-p")==0 && !wavefilepresent){
			wavefilepresent=TRUE;
			strcpy(wavefilename, argv[i+1]);
		}else
		if(strcmp(argv[i], "-in")==0 && !infilepresent){
			infilepresent=TRUE;
			strcpy(infilename, argv[i+1]);
		}else
		if(strcmp(argv[i], "-out")==0 && !outfilepresent){
			outfilepresent=TRUE;
			strcpy(outfilename, argv[i+1]);
		}else
		if(strcmp(argv[i], "-steg")==0 && lsb_method==-1){
			if(strcmp(argv[i+1],"LSB1")==0){
				lsb_method=1;
			}else
			if(strcmp(argv[i+1],"LSB2")==0){
				lsb_method=2;
			}else
			if(strcmp(argv[i+1],"LSB4")==0){
				lsb_method=4;
			}else
			if(strcmp(argv[i+1],"LSBE")==0){
				lsb_method=4;
				lsbe=1;
			}else{
				parsingError();
			}
		}else
		if(strcmp(argv[i], "-a")==0){
			if(strcmp(argv[i+1], "aes128")==0){
				enc_met=AES128;
			}else
			if(strcmp(argv[i+1], "aes256")==0){
				enc_met=AES256;
			}else
			if(strcmp(argv[i+1], "aes192")==0){
				enc_met=AES192;
			}else
			if(strcmp(argv[i+1], "des")==0){
				enc_met=DES;
			}else{
				parsingError();
			}
		}else
		if(strcmp(argv[i], "-m")==0){
			if(strcmp(argv[i+1], "ecb")==0){
				block_met=ECB;
			}else
			if(strcmp(argv[i+1], "cfb")==0){
				block_met=CFB;
			}else
			if(strcmp(argv[i+1], "ofb")==0){
				block_met=OFB;
			}else
			if(strcmp(argv[i+1], "cbc")==0){
				block_met=CBC;
			}else{
				parsingError();
			}
		}else
		if(strcmp(argv[i], "-pass")==0){
			password=malloc(strlen(argv[i+1]));
			strcpy(password, argv[i+1]);
		}else{
			parsingError();
		}
	}

	ptr_wavefile = fopen(wavefilename, "rb");
	if (ptr_wavefile == NULL) {
		printf("Error abriendo el archivo portador\n");
		exit(1);
	}
	
	if(extract){
		if(!wavefilepresent || lsb_method==-1 || !outfilepresent){
			parsingError();
		}
	}else{
		if(!wavefilepresent || lsb_method==-1 || !outfilepresent || !infilepresent){
			parsingError();
		}else{
			out_wavefile=fopen(outfilename, "w");
		}
	}
 
 
 // -- Desde aca es copia de: http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
 int read = 0;
 
 // read header parts

 read = fread(header.riff, sizeof(header.riff), 1, ptr_wavefile);
 if(!extract){
	fwrite(header.riff, sizeof(header.riff), 1, out_wavefile);
 }
 //printf("(1-4): %s \n", header.riff); 

 read = fread(buffer4, sizeof(buffer4), 1, ptr_wavefile);
 if(!extract){
	fwrite(buffer4, sizeof(buffer4), 1, out_wavefile);
 }
 //printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);
 
 // convert little endian to big endian 4 byte int
 header.overall_size  = buffer4[0] | 
						(buffer4[1]<<8) | 
						(buffer4[2]<<16) | 
						(buffer4[3]<<24);

 //printf("(5-8) Overall size: bytes:%u, Kb:%u \n", header.overall_size, header.overall_size/1024);

 read = fread(header.wave, sizeof(header.wave), 1, ptr_wavefile);
 if(!extract){
	fwrite(header.wave, sizeof(header.wave), 1, out_wavefile);
 }
 //printf("(9-12) Wave marker: %s\n", header.wave);

 read = fread(header.fmt_chunk_marker, sizeof(header.fmt_chunk_marker), 1, ptr_wavefile);
 if(!extract){
	fwrite(header.fmt_chunk_marker, sizeof(header.fmt_chunk_marker),1, out_wavefile);
 }
 //printf("(13-16) Fmt marker: %s\n", header.fmt_chunk_marker);

 read = fread(buffer4, sizeof(buffer4), 1, ptr_wavefile);
 if(!extract){
	fwrite(buffer4, sizeof(buffer4), 1, out_wavefile);
 }
 //printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

 // convert little endian to big endian 4 byte integer
 header.length_of_fmt = buffer4[0] |
							(buffer4[1] << 8) |
							(buffer4[2] << 16) |
							(buffer4[3] << 24);
 //printf("(17-20) Length of Fmt header: %u \n", header.length_of_fmt);

 read = fread(buffer2, sizeof(buffer2), 1, ptr_wavefile);
  if(!extract){
	fwrite(buffer2, sizeof(buffer2), 1, out_wavefile);
 }
 //printf("%u %u \n", buffer2[0], buffer2[1]);
 
 header.format_type = buffer2[0] | (buffer2[1] << 8);
 char format_name[10] = "";
 if (header.format_type == 1)
   strcpy(format_name,"PCM"); 
 else if (header.format_type == 6)
  strcpy(format_name, "A-law");
 else if (header.format_type == 7)
  strcpy(format_name, "Mu-law");

 //printf("(21-22) Format type: %u %s \n", header.format_type, format_name);

 read = fread(buffer2, sizeof(buffer2), 1, ptr_wavefile);
 if(!extract){
	fwrite(buffer2, sizeof(buffer2), 1, out_wavefile);
 }
 //printf("%u %u \n", buffer2[0], buffer2[1]);

 header.channels = buffer2[0] | (buffer2[1] << 8);
 //printf("(23-24) Channels: %u \n", header.channels);

 read = fread(buffer4, sizeof(buffer4), 1, ptr_wavefile);
 if(!extract){
	fwrite(buffer4, sizeof(buffer4), 1, out_wavefile);
 }
 //printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

 header.sample_rate = buffer4[0] |
						(buffer4[1] << 8) |
						(buffer4[2] << 16) |
						(buffer4[3] << 24);

 //printf("(25-28) Sample rate: %u\n", header.sample_rate);

 read = fread(buffer4, sizeof(buffer4), 1, ptr_wavefile);
  if(!extract){
	fwrite(buffer4, sizeof(buffer4), 1, out_wavefile);
 }
 //printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

 header.byterate  = buffer4[0] |
						(buffer4[1] << 8) |
						(buffer4[2] << 16) |
						(buffer4[3] << 24);
 //printf("(29-32) Byte Rate: %u , Bit Rate:%u\n", header.byterate, header.byterate*8);

 read = fread(buffer2, sizeof(buffer2), 1, ptr_wavefile);
  if(!extract){
	fwrite(buffer2, sizeof(buffer2), 1, out_wavefile);
 }
 //printf("%u %u \n", buffer2[0], buffer2[1]);

 header.block_align = buffer2[0] |
					(buffer2[1] << 8);
 //printf("(33-34) Block Alignment: %u \n", header.block_align);

 read = fread(buffer2, sizeof(buffer2), 1, ptr_wavefile);
   if(!extract){
	fwrite(buffer2, sizeof(buffer2), 1, out_wavefile);
 }
 //printf("%u %u \n", buffer2[0], buffer2[1]);

 header.bits_per_sample = buffer2[0] |
					(buffer2[1] << 8);
 //printf("(35-36) Bits per sample: %u \n", header.bits_per_sample);

 read = fread(header.data_chunk_header, sizeof(header.data_chunk_header), 1, ptr_wavefile);
   if(!extract){
	fwrite(header.data_chunk_header, sizeof(header.data_chunk_header), 1, out_wavefile);
 }
 //printf("(37-40) Data Marker: %s \n", header.data_chunk_header);

 read = fread(buffer4, sizeof(buffer4), 1, ptr_wavefile);
 if(!extract){
	fwrite(buffer4, sizeof(buffer4), 1, out_wavefile);
 }
 //printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

 header.data_size = buffer4[0] |
				(buffer4[1] << 8) |
				(buffer4[2] << 16) | 
				(buffer4[3] << 24 );
 //printf("(41-44) Size of data chunk: %u \n", header.data_size);


 // calculate no.of samples
 long num_samples = (8 * header.data_size) / (header.channels * header.bits_per_sample);
 //printf("Number of samples:%lu \n", num_samples);

 long size_of_each_sample = (header.channels * header.bits_per_sample) / 8;
 //printf("Size of each sample:%ld bytes\n", size_of_each_sample);

 /*// calculate duration of file
 float duration_in_seconds = (float) header.overall_size / header.byterate;
 printf("Approx.Duration in seconds=%f\n", duration_in_seconds);
 printf("Approx.Duration in h:m:s=%s\n", seconds_to_time(duration_in_seconds));*/

// -- Hasta aca
	FILE* in_file;
	BYTE* in_data;
	unsigned long in_file_size;
 	if(extract){
		out_data_size = (num_samples/8)*lsb_method;
		out_data = (BYTE*)calloc(out_data_size, sizeof(BYTE));
	}else{
		in_file = fopen(infilename, "r");
		fseek(in_file, 0L, SEEK_END);
		in_file_size = ftell(in_file);
		rewind(in_file);
		in_data = (BYTE*)malloc(in_file_size+4+strlen(infilename));
		if(in_data==NULL){
			printf("El archivo a embeber es muy grande");
			exit(1);
		}
		memcpy(in_data, &in_file_size, 4);
		in_data[0] = INT_TO_BYTE(in_file_size, 0);
		in_data[1] = INT_TO_BYTE(in_file_size, 1);
		in_data[2] = INT_TO_BYTE(in_file_size, 2);
		in_data[3] = INT_TO_BYTE(in_file_size, 3);
		unsigned int out_file_size =
						(in_data[0] << 24) |
						(in_data[1] << 16) |
						(in_data[2] << 8) |
						(in_data[3]);
		printf("size of file to embed %d\n", out_file_size);
		fread(in_data+4, in_file_size, 1, in_file);
		char *ptr_in = infilename;
		while(*ptr_in!='.'){
			ptr_in++;
		}
		long ptr=4+in_file_size;
		while(*ptr_in!=0){
			in_data[ptr]=*ptr_in;
			ptr++;
			ptr_in++;
		}
		in_data[ptr]='\0';
		in_file_size=ptr+1;	
		if(password!=NULL){		
			BYTE* aux=malloc(in_file_size+4);
			unsigned long new_size = encrypt_wrapper(in_data, in_file_size, (BYTE*)password, aux+4, enc_met, block_met);
			free(in_data);
			in_data=aux;
			in_data[0] = INT_TO_BYTE(new_size, 0);
			in_data[1] = INT_TO_BYTE(new_size, 1);
			in_data[2] = INT_TO_BYTE(new_size, 2);
			in_data[3] = INT_TO_BYTE(new_size, 3);
			in_file_size = new_size+4;
			unsigned int out_file_size =
						(in_data[0] << 24) |
						(in_data[1] << 16) |
						(in_data[2] << 8) |
						(in_data[3]);
			printf("size of file encrypted %d\n", out_file_size);
	
		}
	}
	
	int k=0; //Contador auxiliar, por ejemplo para el lsbe
	// read each sample from data chunk if PCM
	if(header.format_type == 1) { // PCM 
		BYTE data_buffer[size_of_each_sample];
		for (i = 0; i < num_samples; i++) {
			read = fread(data_buffer, sizeof(data_buffer), 1, ptr_wavefile);
			if (read == 1) {
				if(lsbe){
					long j;
					for(j=0; j<size_of_each_sample; j++){
						if(data_buffer[j] == 0xFE || data_buffer[j] == 0xFF){
							if(extract){
								getLSB1(out_data, k++,data_buffer[j]);
							}else{
								if(k/8<in_file_size){
									addLSB1(in_data, k++, data_buffer+j);
								}
							}
						}
					}
				}else{
					BYTE* lsb = data_buffer+(size_of_each_sample-1); // LSB
					if(extract){
						switch(lsb_method){
							case 1: 
								getLSB1(out_data, i, *lsb);
								break;
							case 2:
								getLSB2(out_data, i, *lsb);
								break;
							case 4:
								getLSB4(out_data, i, *lsb);
								break;
							default:
								break;
						}
					}else{
						switch(lsb_method){
							case 1: 
								if(k/8<in_file_size){
									addLSB1(in_data, k++, lsb);
								}
								break;
							case 4:
								if(k/8<=in_file_size){
									addLSB4(in_data, k, lsb);
									k+=4;
								}
								break;
							default:
								break;
						}
						
					}
				}
				
			}
			else {
				printf("Error parseando el .wav\n");
				exit(1);
			}
			if(!extract) {
				fwrite(data_buffer, sizeof(data_buffer), 1, out_wavefile);
			}
		} 
	}else{
		printf("Error parseando el .wav\n");
		exit(1);
	}
	
	if(extract){
		unsigned long out_file_size =
						(out_data[0] << 24) |
						(out_data[1] << 16) |
						(out_data[2] << 8) |
						(out_data[3]);
		if(password){
			BYTE *ans = malloc(out_file_size);
			printf("size before decrypted: %lu\n", out_file_size);
			decrypt_wrapper(out_data+4, out_file_size, (BYTE*)password, ans, enc_met, block_met);
			free(out_data);
			out_data = ans;
			out_file_size =
							(out_data[0] << 24) |
							(out_data[1] << 16) |
							(out_data[2] << 8) |
							(out_data[3]);
		}
		printf("size: %lu\n", out_file_size);
		unsigned long i = 0;
		char ext[128];
		int offset = 4;
		do{
			if(i+(offset+out_file_size) > out_data_size){
				printf("Error al extraer\n");
				return 0;
			}
			if(i>128){
				printf("Extension no correcta\n");
				return 0;
			}
			ext[i]=out_data[i+(offset+out_file_size)];
		}while(out_data[(i++)+(offset+out_file_size)]);
		if(ext[0]!='.'){
			printf("Extension no correcta\n");
			return 0;
		}
		printf("ext: %s\n", ext);
		FILE *f = fopen(strcat(outfilename, ext), "w");
		fwrite(out_data+offset, 1, out_file_size, f);
		fclose(f);
	}else{
		if(k/8<in_file_size){
			printf("No se pude embeber el archivo en el .wav\n");
			printf("El .wav portador puede albergar %d bytes\n", k/8);
			fclose(out_wavefile);
			remove(outfilename);
		}else{
			printf("Archivo embebido con exito\n");
			fclose(out_wavefile);
		}
	}
	
	fclose(ptr_wavefile);
	return 0;
}



