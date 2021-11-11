#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#define MEMSIZ 4096

//Defino estructura del chip-8
struct machine_t {
	uint8_t mem[MEMSIZ];	// Memoria del chip-8 de tamaño MEMSIZ
	uint16_t pc;			// Program Counter

	uint16_t stack[16];		//Pila del chip-8
	uint16_t sp;

	uint8_t V[16];
	uint16_t i;
	uint8_t dt, st;
};

//Función para inicializar máquina a 0
void init_machine(struct machine_t* machine){
	machine->sp = machine->i = machine->dt = machine->st = 0x00;

	machine->pc = 0x200;

	for(int i = 0; i < MEMSIZ; i++){
		machine->mem[i] = 0x00;
	}
	for(int i = 0; i < 16; i++){
		machine->stack[i] = 0x00;
		machine->V[i] = 0x00;
	}
}

// Lectura de ROMs
void load_rom(struct machine_t* machine){
	FILE* fp = fopen("PONG", "r");

	if(fp==NULL){
		fprintf(stderr, "Cannot open ROM file.\n");
		exit(1);
	}

	fseek(fp, 0, SEEK_END);		// Hallo el tamaño de fp colocándome al final del
	int length = ftell(fp);		// fichero y asignando la posición a la variable
	fseek(fp, 0, SEEK_SET);		// length. Luego vuelvo a posición inicial

	fread(machine->mem + 0x200, length, 1, fp);

	fclose(fp);
}

void clean(){
	SDL_Quit();
}

int main(int argc, char** argv){

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow(	"CHIP-8",
											SDL_WINDOWPOS_CENTERED,
											SDL_WINDOWPOS_CENTERED,
											640,
											320,
											SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if(window==NULL){
		printf("Error: La ventana no pudo ser creada");
	} else {
		SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
													SDL_TEXTUREACCESS_STREAMING,
													64, 32);

		int pitch;
		Uint32* pixels;
		SDL_LockTexture(texture, NULL, (void**) &pixels, &pitch);
		// memset
		memset(pixels, 0x80, 32 * pitch);

		SDL_UnlockTexture(texture);

		int isRunning=1;

		while(isRunning){
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);

			SDL_Event e;
			while(SDL_PollEvent(&e)){
				if(e.type==SDL_QUIT){
					isRunning = 0;
				}
			}
		}
	}



	clean();
	return 0;
}


int decode(){
	struct machine_t mac;
	init_machine(&mac);
	load_rom(&mac);

	int mustQuit=0;

	while(!mustQuit){
		// Leer el opcode
		uint16_t opcode = (mac.mem[mac.pc]<<8) | mac.mem[mac.pc + 1];

		if((mac.pc+=2) == MEMSIZ){
			mac.pc = 0x200;
			mustQuit = 1;
		}

		uint16_t nnn = opcode & 0x0FFF;
		uint8_t kk = opcode & 0xFF;
		uint8_t n = opcode & 0xF;
		uint8_t x = (opcode >> 8) & 0xF;
		uint8_t y = (opcode >> 4) & 0xF;
		uint8_t p = (opcode >> 12);

		switch(p) {
			case 0:
				if(opcode==0x00E0){
					// TODO: CLS
					printf("CLS\n");
				} else if (opcode==0x00EE){
					// TODO: RET
					printf("RET\n");
				}
				break;
			case 1:
				// TODO: JP
				printf("JP %x\n", nnn);
				mac.pc = nnn;
				break;
			case 2:
				// TODO: CALL
				printf("CALL %x\n", nnn);
				break;
			case 3:
				// TODO: SE
				printf("SE %x %x\n", x, kk);
				if(mac.V[x]==kk){
					mac.pc = (mac.pc + 2) & 0xFFF;
				}
				break;
			case 4:
				//TODO: SNE
				printf("SNE %x %x\n", x, kk);
				if(mac.V[x]==kk){
					mac.pc = (mac.pc + 2) & 0xFFF;
				}
				break;
			case 5:
				//printf("SE %x %x\n", x, y);
				if(mac.V[x]==mac.V[y]){
					mac.pc = (mac.pc + 2) & 0xFFF;
				}
				break;
			case 6:
				//printf("LD %x %x\n", x, kk);
				mac.V[x] = kk;
				break;
			case 7:
				//printf("ADD %x %x\n", x, kk);
				mac.V[x] = (mac.V[x] + kk) & 0xFF;
				break;
			case 8:
				switch(n){
					case 0:
						//printf("LD %x %x\n", x, y);
						mac.V[x] = mac.V[y];
						break;
					case 1:
						//printf("OR %x %x\n", x, y);
						mac.V[x] |= mac.V[y];
						break;
					case 2:
						//printf("AND %x %x\n", x, y);
						mac.V[x] &= mac.V[y];
						break;
					case 3:
						//printf("XOR %x %x\n", x, y);
						mac.V[x] ^= mac.V[y];
						break;
					case 4:
						//printf("ADD %x %x\n", x, y);
						mac.V[0xF] = (mac.V[x] > mac.V[x] + mac.V[y]);
						mac.V[x] = (mac.V[x] + mac.V[y]) & 0xFF;
						break;
					case 5:
						//printf("SUB %x %x\n", x, y);
						mac.V[0xF] = (mac.V[x] > mac.V[y]);
						mac.V[x] = (mac.V[x] - mac.V[y]) & 0xFF;
						break;
					case 6:
						printf("SHR %x %x\n", x, y);
						mac.V[0xF] = (mac.V[x] & 0x01);
						mac.V[x] = mac.V[x] >> 1;
						break;
					case 7:
						printf("SUBN %x %x\n", x, y);
						mac.V[0xF] = (mac.V[x] < mac.V[y]);
						mac.V[x] = (mac.V[y] - mac.V[x]) & 0xFF;
						break;
					case 0xE:
						printf("SHL %x %x\n", x, y);
						mac.V[0xF] = (mac.V[x] & 0x80) != 0;
						mac.V[x] = (mac.V[x] << 1) & 0xFF;
						break;
				}
				break;
			case 9:
				printf("SNE %x %x\n", x, y);
				if(mac.V[x]!=mac.V[y]){
					mac.pc = (mac.pc+2) & 0xFFF;
				}
				break;
			case 0xA:
				printf("LD %x\n", nnn);
				mac.i = nnn;
				break;
			case 0xB:
				printf("JP %x\n", nnn);
				mac.pc = nnn + mac.V[0];
				break;
			case 0xC:
				printf("RND %x %x\n", x, kk);
				
				break;
			case 0xD:
				printf("DRW %x %x %x\n", x, y, n);
				break;
			case 0xE:
				if(kk==0x9E){
					printf("SKP %x\n", x);
				} else if(kk == 0xA1){
					printf("SKNP %x\n", x);
				}
				break;
			case 0xF:
				switch(kk){
					case 0x07:
						printf("LD %x, DT\n", x);
						mac.V[x] = mac.dt;
						break;
					case 0x0A:
						printf("LD %x, K\n", x);

						break;
					case 0x15:
						printf("LD DT, %x\n", x);
						mac.dt = mac.V[x];
						break;
					case 0x18:
						printf("LD ST, %x\n", x);
						mac.st = mac.V[x];
						break;
					case 0x1E:
						printf("ADD I, %x\n", x);
						mac.i = mac.i + mac.V[x];
						break;
					case 0x29:
						printf("LD F, %x\n", x);

						break;
					case 0x33:
						printf("LD B, %x\n", x);
						break;
					case 0x55:
						printf("LD [I] %x\n", x);

						break;
					case 0x65:
						printf("LD %x, [I]\n", x);
						break;
				}
				break;
		}

		for(int i=0; i<16; i++)
			printf("El registro V%d vale %x\n", i,mac.V[1]);
	}


	//printf("%x", mac.mem[0x200]);

	printf("¡Hola mundo!\n");
	return 0;
}

