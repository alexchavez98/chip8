#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>

#define MEMSIZ        4096
#define CHIP8_WIDTH   64
#define CHIP8_HEIGHT  32
#define WINDOW_WIDTH  CHIP8_WIDTH*10
#define WINDOW_HEIGHT CHIP8_HEIGHT*10

unsigned char fontset[80]={
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

//Defino estructura del chip-8
typedef struct machine_t {
	uint8_t mem[MEMSIZ];	// Memoria del chip-8 de tamaño MEMSIZ
	uint16_t pc;			// Program Counter

	uint16_t stack[16];		//Pila del chip-8
	uint16_t sp;

	uint8_t V[16];
	uint16_t i;
	uint8_t dt, st;

    char screen[CHIP8_WIDTH*CHIP8_HEIGHT];
} Chip8;

static void expansion(char* from, Uint32* to){
	for(int i=0; i<2048; i++){
		to[i] = (from[i]) ? -1 : 0;
	}
}

void init_machine(Chip8* machine);
void load_rom(Chip8* machine);
int decode();
void draw(Chip8* cpu);

//Función para inicializar máquina a 0
void init_machine(Chip8* machine){
	machine->sp = machine->i = machine->dt = machine->st = 0x00;

	machine->pc = 0x200;

	for(int i = 0; i < MEMSIZ; i++){
		machine->mem[i] = 0x00;
	}
	for(int i = 0; i < 16; i++){
		machine->stack[i] = 0x00;
		machine->V[i] = 0x00;
	}
        for(int i = 0; i < 80; i++){
          machine-> mem[i] = fontset[i];
        }

    for(int i = 0; i < 2048; i++){
          machine-> screen[i] = 0;
        }
}

// Lectura de ROMs
void load_rom(Chip8* machine){
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

int lastTicks;

int main(int argc, char** argv){

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Surface* surface;


	//CHIP8 configuration
	Chip8 chip8;
	init_machine(&chip8);
	load_rom(&chip8);
	srand(time(NULL));


	//for(int i=0; i<2048; i++){
	//	chip8.screen[i] =(rand() & 1);
	//}
	int lastTicks = SDL_GetTicks();

	//SDL Configuration
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow(	"CHIP-8",
											SDL_WINDOWPOS_CENTERED,
											SDL_WINDOWPOS_CENTERED,
											WINDOW_WIDTH,
											WINDOW_HEIGHT,
											SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if(window==NULL){
		printf("Error: La ventana no pudo ser creada");
	} else {
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
						        SDL_TEXTUREACCESS_STREAMING,
						      	CHIP8_WIDTH, CHIP8_HEIGHT);

		surface = SDL_CreateRGBSurface(0, 64, 32, 32,
			0x00FF0000,
			0x0000FF00,
			0x000000FF,
			0xFF000000);

		int pitch;
		Uint32* pixels;
		//SDL_LockTexture(texture, NULL, &surface->pixels, &surface->pitch);
		// memset
		printf("%d", surface->pitch);
		//memset(pixels, 0x80, 32 * pitch);

		//expansion(chip8.screen, (Uint32*) surface->pixels);

		//SDL_UnlockTexture(texture);

		int isRunning=1;

        

		while(isRunning){
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);

			if(chip8.dt>0)
				chip8.dt-=1;
			if(chip8.st>0)
				chip8.dt-=1;

			decode(&chip8);

			SDL_Event e;
			while(SDL_PollEvent(&e)){
				if(e.type==SDL_QUIT){
					isRunning = 0;
				}
			}

			SDL_LockTexture(texture, NULL, &surface->pixels, &surface->pitch);
			// memset
			printf("%d", surface->pitch);
			//memset(pixels, 0x80, 32 * pitch);

			expansion(chip8.screen, (Uint32*) surface->pixels);

			SDL_UnlockTexture(texture);

			if(SDL_GetTicks() < lastTicks + 1000/2.0){
				SDL_Delay(-SDL_GetTicks()+lastTicks+1000/2.0);
				//isRunning = 0;
			}
			lastTicks = SDL_GetTicks();
		}
	}



	clean();
	return 0;
}


int decode(Chip8* cpu){

	int mustQuit=0;

	//while(!mustQuit){
		mustQuit =1;
		// Leer el opcode
		uint16_t opcode = (cpu->mem[cpu->pc]<<8) | cpu->mem[cpu->pc + 1];

		if((cpu->pc+=2) == MEMSIZ){
			cpu->pc = 0x200;
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
                    memset(cpu->screen, 0, sizeof(cpu->screen));
                                        
				} else if (opcode==0x00EE){
					// TODO: RET
					printf("RET\n");
					cpu->sp =(cpu->sp-1) & 0xFFFF;
					cpu->pc = cpu->stack[cpu->sp];
				}
				break;
			case 1:
				// TODO: JP
				printf("JP %x\n", nnn);
				cpu->pc = nnn;
				break;
			case 2:
				// TODO: CALL
				printf("CALL %x\n", nnn);
				cpu->sp = (cpu->sp+1) & 0xFFFF;
				cpu->pc = nnn;
				break;
			case 3:
				// TODO: SE
				printf("SE %x %x\n", x, kk);
				if(cpu->V[x]==kk){
					cpu->pc = (cpu->pc + 2) & 0xFFF;
				}
				break;
			case 4:
				//TODO: SNE
				printf("SNE %x %x\n", x, kk);
				if(cpu->V[x]==kk){
					cpu->pc = (cpu->pc + 2) & 0xFFF;
				}
				break;
			case 5:
				printf("SE %x %x\n", x, y);
				if(cpu->V[x]==cpu->V[y]){
					cpu->pc = (cpu->pc + 2) & 0xFFF;
				}
				break;
			case 6:
				printf("LD V[%x] %x\n", x, kk);
				cpu->V[x] = kk;
				break;
			case 7:
				printf("ADD %x %x\n", x, kk);
				cpu->V[x] = (cpu->V[x] + kk) & 0xFF;
				break;
			case 8:
				switch(n){
					case 0:
						printf("LD V%x V%x\n", x, y);
						cpu->V[x] = cpu->V[y];
						break;
					case 1:
						printf("OR %x %x\n", x, y);
						cpu->V[x] |= cpu->V[y];
						break;
					case 2:
						printf("AND %x %x\n", x, y);
						cpu->V[x] &= cpu->V[y];
						break;
					case 3:
						printf("XOR %x %x\n", x, y);
						cpu->V[x] ^= cpu->V[y];
						break;
					case 4:
						printf("ADD %x %x\n", x, y);
						cpu->V[0xF] = (cpu->V[x] > cpu->V[x] + cpu->V[y]);
						cpu->V[x] = (cpu->V[x] + cpu->V[y]) & 0xFF;
						break;
					case 5:
						printf("SUB %x %x\n", x, y);
						cpu->V[0xF] = (cpu->V[x] > cpu->V[y]);
						cpu->V[x] = (cpu->V[x] - cpu->V[y]) & 0xFF;
						break;
					case 6:
						printf("SHR %x %x\n", x, y);
						cpu->V[0xF] = (cpu->V[x] & 0x01);
						cpu->V[x] = cpu->V[x] >> 1;
						break;
					case 7:
						printf("SUBN %x %x\n", x, y);
						cpu->V[0xF] = (cpu->V[x] < cpu->V[y]);
						cpu->V[x] = (cpu->V[y] - cpu->V[x]) & 0xFF;
						break;
					case 0xE:
						printf("SHL %x %x\n", x, y);
						cpu->V[0xF] = (cpu->V[x] & 0x80) != 0;
						cpu->V[x] = (cpu->V[x] << 1) & 0xFF;
						break;
				}
				break;
			case 9:
				printf("SNE %x %x\n", x, y);
				if(cpu->V[x]!=cpu->V[y]){
					cpu->pc = (cpu->pc+2) & 0xFFF;
				}
				break;
			case 0xA:
				printf("LD %x\n", nnn);
				cpu->i = nnn & 0xFFF;
				break;
			case 0xB:
				printf("JP %x\n", nnn);
				cpu->pc = (nnn + cpu->V[0]) & 0xFFF;
				break;
			case 0xC:
				printf("RND %x %x\n", x, kk);
				
				break;
			case 0xD:
				printf("DRW %x %x %x\n", x, y, n);
				for(int j=0; j<n; j++){
					uint8_t sprite = cpu->mem[cpu->i];
					for(int i=0;i<7;i++){
						int px = (cpu->V[x] + i) & 63;
						int py = (cpu->V[y] + j) & 31;
						cpu->screen[py*64 + px] = (sprite & (1<<(7-i)))!=0;
						
					}
				}
				printf("%x\n",cpu->V[0xA]);
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
						cpu->V[x] = cpu->dt;
						break;
					case 0x0A:
						printf("LD %x, K\n", x);
						//cpu->i = cpu->V[x];
						break;
					case 0x15:
						printf("LD DT, %x\n", x);
						cpu->dt = cpu->V[x];
						break;
					case 0x18:
						printf("LD ST, %x\n", x);
						cpu->st = cpu->V[x];
						break;
					case 0x1E:
						printf("ADD I, %x\n", x);
						cpu->i = cpu->i + cpu->V[x];
						break;
					case 0x29:
						printf("LD F, %x\n", x);
						cpu->i=cpu->V[x]*0x5;
						break;
					case 0x33:
						printf("LD B, %x\n", x);
						cpu->mem[cpu->i] = cpu->V[x]/100;
						cpu->mem[cpu->i+1] = cpu->V[x]/10;
						cpu->mem[cpu->i+2] =cpu->V[x] %10;
						break;
					case 0x55:
						printf("LD [I] %x\n", x);
						for(int i = 0; i<x;i++){
							cpu->mem[cpu->i+i] = cpu->V[i];
						}
						break;
					case 0x65:
						printf("LD %x, [I]\n", x);
						for(int i = 0; i<x;i++){
							cpu->V[i] = cpu->mem[cpu->i+i];
						}
						break;
				}
				break;
		}

		//for(int i=0; i<16; i++)
		//	printf("El registro V%d vale %x\n", i,cpu.V[1]);
	//}


	//printf("%x", cpu.mem[0x200]);

	return 0;
}

void draw(Chip8* cpu){
  //SDL_RenderClear(renderer);
}

