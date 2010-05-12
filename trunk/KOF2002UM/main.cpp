//==========================================================================
/*
autor: brbranco
data de inicio 10/04/2010
objetivo: controlar o jogo kof2002um
usando o emulador pcsx2 rev=r1888
*/
//==========================================================================

#include <allegro.h> 
#include <winalleg.h>
#include <Mmsystem.h>				//som e joystick
#include <Autoit3.h>				//envio de teclas
#include <stdio.h>					//para os arquivos
#include "funcoes.h"				// contem os prototipos das funçoes utilizadas
#include "overlay_renderer.h"
#include "player.h"
#include <string>

//definição dos valores das telas
#define TELA_MENU1_MIN 8000
#define TELA_MENU1_MAX 11000
#define TELA_SELECT_MIN 420
#define TELA_SELECT_MAX 450
#define TELA_ORDENA_CHAR_MIN 336
#define TELA_ORDENA_CHAR_MAX 364
#define TELA_INICIO_LUTA 1000
#define TELA_APRESENTACAO 4
#define TELA_ASSINA_MIN 291
#define TELA_ASSINA_MAX 296
#define TELA_CONTINUE_MIN 340
#define TELA_CONTINUE_MAX 382
#define TELA_MEMORY_CARD1 842
#define TELA_MEMORY_CARD2 848

//macros utilizadas
#define ID_GAMEKILLER_1 9998			//define o id do timer que lê o estado dos joysticks
#define TIMER1_TEMPO (1000/30)			//define o tempo de resposta de cada timer
#define TEMPOAGUARDA 25000				//define o tempo de espera por uma tela
#define TITULOJANELA "BRKOF2002UM"		//titulo da janela
#define MAX_BUFF_MENSA 130				//define o numero maximo de caracteres, usados por "sprintf_s"
#define MAX_BUFF_INI 100				//define o tamanho do buffer para leitura de arquivos .ini
#define CONFIG_INI ".\\Brconf_memo.ini"	//nome do arquivo .ini de configuração


//macros para facilitar o envio de teclas
//player1 {start1 /, botão B *}, player2{start2 -, botão B +}
//isso porque nos jogos japoneses o {bolinha é quem ativa uma opção}
#define ENVIA1_A AU3_Send(L"{1 DOWN}", 0);Sleep(100);AU3_Send(L"{1 UP}", 0) //X
#define ENVIA1_B AU3_Send(L"{2 DOWN}", 0);Sleep(100);AU3_Send(L"{2 UP}", 0) //bolinha
#define ENVIA1_X AU3_Send(L"{3 DOWN}", 0);Sleep(100);AU3_Send(L"{3 UP}", 0)	//quadrado
#define ENVIA1_Y AU3_Send(L"{4 DOWN}", 0);Sleep(100);AU3_Send(L"{4 UP}", 0)	//triangulo
#define ENVIA1_S AU3_Send(L"{5 DOWN}", 0);Sleep(100);AU3_Send(L"{5 UP}", 0) //start1
#define ENVIA1_R AU3_Send(L"{R DOWN}", 0);Sleep(100);AU3_Send(L"{R UP}", 0) //direita

//PLAYER2
#define ENVIA2_A AU3_Send(L"{6 DOWN}", 0);Sleep(100);AU3_Send(L"{6 UP}", 0) //X
#define ENVIA2_B AU3_Send(L"{7 DOWN}", 0);Sleep(100);AU3_Send(L"{7 UP}", 0) //bolinha
#define ENVIA2_X AU3_Send(L"{8 DOWN}", 0);Sleep(100);AU3_Send(L"{8 UP}", 0)	//quadrado
#define ENVIA2_Y AU3_Send(L"{9 DOWN}", 0);Sleep(100);AU3_Send(L"{9 UP}", 0)	//triangulo
#define ENVIA2_S AU3_Send(L"{0 DOWN}", 0);Sleep(100);AU3_Send(L"{0 UP}", 0) //start2

#define ENVIA1_F9 AU3_Send(L"{A DOWN}", 0);Sleep(100);AU3_Send(L"{A UP}", 0)	//desabilita xbox360emu


//******************************************************************************
//variaveis globais
//******************************************************************************
JOYINFO joy1;				//estrutura para o joystick 1
JOYINFO joy2;				//estrutura para o joystick 2


HWND meu_hWnd = NULL;		//usado para criar o timer

int JOYSTICKID_1 = 0;									//conterá os numeros dos joysticks
int JOYSTICKID_2 = 0;									//conterá os numeros dos joysticks
int fichas = 0;											//armazena as fichas depositadas
int credito_x = 0, credito_y = 0, credito_t= 0, credito_o=0; //irá conter as coordenadas da menssagens {CREDITO}
BOOL sair = FALSE;										//flags para controlar a saida

//novas variaveis globais para manipular os joystick
//PLAYER1
volatile BOOL FLAG1_A = FALSE, PRESS1_A = FALSE;
volatile BOOL FLAG1_B = FALSE, PRESS1_B = FALSE;
volatile BOOL FLAG1_X = FALSE, PRESS1_X = FALSE;
volatile BOOL FLAG1_Y = FALSE, PRESS1_Y = FALSE;
volatile BOOL FLAG1_S = FALSE, PRESS1_S = FALSE;
//PLAYER2
volatile BOOL FLAG2_A = FALSE, PRESS2_A = FALSE;
volatile BOOL FLAG2_B = FALSE, PRESS2_B = FALSE;
volatile BOOL FLAG2_X = FALSE, PRESS2_X = FALSE;
volatile BOOL FLAG2_Y = FALSE, PRESS2_Y = FALSE;
volatile BOOL FLAG2_S = FALSE, PRESS2_S = FALSE;

volatile BOOL FLAG1_C = FALSE;						//para as fichas joy1
volatile BOOL FLAG2_C = FALSE;						//para as fichas joy2
volatile BOOL FLAG1_Z = FALSE;						//para saida


enum CORES cor_credito;								//define a cor da menssagem
enum BOTOES BT_SAIR;								//numero dos botoes

//ESTÃO SENDO USADOS
DWORD TEMPO_SELECIONA_CHAR=0;
UCHAR NIVEL_DIFICULDADE=0;
UCHAR MAX_POWER=0;
UCHAR MAX_MUSICAS=0;
UCHAR MAX_VOZES=0;
int CREDITOS_POR_FICHA=0;							//multiplicador de creditos
int VOLUME_DEMO=0, VOLUME_JOGO=0;					//amplitude do som
overlay_renderer_t renderer;						//declara o objeto
Player PLAYER1;										//declaras os				
Player PLAYER2;										//dois objetos

//******************************************************************************
//								função main
//embora não aconselhado para nivel de inicialização do código foram
//usados muitos "goto" isso ao meu ver permitiu uma organização mais
//rápidas das funções, isso poderá ser mudado depois dos testes
//******************************************************************************
int main(int argc, char *argv[]){
	UCHAR pedido_entrada=0;		//usado para saber qual player quer entrar
	UCHAR resultado_espera1=0;	//pega o valor de retorno na seleçaõ do player1
	UCHAR resultado_espera2=0;	//pega o valor de retorno na seleção do player2
	UCHAR resultado_espera3=0;	//pega o valor de retorno na seleção do player1 e player2
	UCHAR resultado_luta=0;		//pega o resultado de uma luta

	//seta o tipo de procura do autoiT
	AU3_Opt(L"WinTitleMatchMode", 4);
	
	//deixa os dois player zerados
	PLAYER1.zera_tudo();
	PLAYER2.zera_tudo();
	
    //inicia a bliblioteca allegro
    if(allegro_init() != 0){
		MessageBox(NULL,"Erro ao iniciar a biblioteca allegro","Erro", 0);
        return 0;
	}
            
    //cria a janela
    if(set_gfx_mode(GFX_AUTODETECT_WINDOWED, 180, 120, 0, 0) != 0) {
        allegro_message("Nao foi possivel definir a janela.\n%s\n", allegro_error);
        return 0;
    }
    set_window_title(TITULOJANELA);
	
	//carrega as configurações dos joysticks
    if(!configura_joysticks()){
		return 0;
	}

    //carrega as configurações do arquivo ".ini", dlls e chama o jogo
    if(!carrega_configuracao()){		
		goto  _erros_tratados;
    }
    
    //pega hwnd da janela para ser usado no timer
    meu_hWnd = win_get_window();
    if(meu_hWnd == NULL){
        allegro_message("Erro:1003 Nao foi possivel pegar o handler dessa janela.\n");
        goto  _erros_tratados;
    }

    //instala o timer para ler o estado dos joysticks 30 vezes por segundo.
	if(!SetTimer(meu_hWnd, ID_GAMEKILLER_1, TIMER1_TEMPO, (TIMERPROC)TimerProc)){
		allegro_message("Erro:1004 iniciando o timer.");
		goto  _erros_tratados;
	}
	
	//altera as configurações do xbox360emu.ini
	desabilita_joy(3);
	
ponto_inicial:	//label quando o jogo for reiniciado
	
	//ajusta o volume para demo
	ajusta_volume_som(VOLUME_DEMO);

	//aguarda até uma ficha ser depositada 
	//e o start1 ou start2 ser pressionado
	pedido_entrada=aguarda_deposito_ficha();

	//ajusta o volume e avisa
	ajusta_volume_som(VOLUME_JOGO);
	PlaySound(".\\soms\\inicio.wav", 0, SND_FILENAME | SND_ASYNC);

	//verifica quem pediu para entrar
	switch(pedido_entrada){
		case 1:
			goto ponto_player1;
			break;
		case 2:
			goto ponto_player2;
			break;
		default:
			goto _erros_tratados;
	}

ponto_player1:	//player1 vS CPU
	//posiciona no menu principal e seleciona a opção team selection
	//ajusta o nivel de dificulade eo max_power
	resultado_espera1=seleciona_modo_inicial(1);
	if(!resultado_espera1){
		goto _erros_tratados;
	}

ponto_continue_player1:
	//agora espera até que o player seja selecionado
	resultado_espera1=espera_selecionar_player1();
	switch(resultado_espera1){
		case 1:
			goto ponto_lutando1;
			break;
		case 2:
			goto ponto_versus;
			break;
		default:
			goto _erros_tratados;
	}

ponto_player2:	//player2 VS CPU
	//posiciona no menu principal e seleciona a opção team selection
	resultado_espera2=seleciona_modo_inicial(2);
	if(!resultado_espera2){
		goto _erros_tratados;
	}

ponto_continue_player2:
	//espera até que o player seja selecionado
	resultado_espera2=espera_selecionar_player2();
	switch(resultado_espera2){
		case 1:
			goto ponto_lutando2;
			break;
		case 2:
			goto ponto_versus;
			break;
		default:
			goto _erros_tratados;
	}


ponto_versus:	//versus
	resultado_espera3=espera_selecionar_player3();
	if(resultado_espera3){
		goto ponto_lutando3;
	}else{
		goto _erros_tratados;
	}

ponto_lutando1:
	//faz um delay até que chegue na tela de luta
	resultado_luta=espera_tela_inicio_luta();
	if(!resultado_luta){
		goto _erros_tratados;
	}

	resultado_luta=espera_terminar_jogo_p1();
	switch(resultado_luta){
		case 0:					//termino normal
			goto ponto_inicial;
			break;
		case 1:					//pedido de continue do player1
			goto ponto_continue_player1;
			break;
		case 2:					//pedido de entrada do player2
			goto ponto_versus;
			break;
		default:				//erro
			goto _erros_tratados;		
	}


ponto_lutando2:
	//faz um delay até que chegue na tela de luta
	resultado_luta=espera_tela_inicio_luta();
	if(!resultado_luta){
		goto _erros_tratados;
	}

	resultado_luta=espera_terminar_jogo_p2();
	switch(resultado_luta){
		case 0:					//termino normal
			goto ponto_inicial;
			break;
		case 1:					//pedido de continue do player2
			goto ponto_continue_player2;
			break;
		case 2:					//pedido de entrada do player1
			goto ponto_versus;
			break;
		default:				//erro
			goto _erros_tratados;		
	}


ponto_lutando3:
	resultado_luta=espera_terminar_jogo_p3();
	switch(resultado_luta){
		case 1:
			goto ponto_continue_player1;	//player1 ganhou
			break;
		case 2:
			goto ponto_continue_player2;	//player2 ganhou
			break;
		case 3:
			goto ponto_inicial;				//empate
			break;
		default:
			goto _erros_tratados;
	}


_erros_tratados:		
		ajusta_volume_som(VOLUME_JOGO);
		habilita_joy(3);

        //envia um {ESC} para fechar o GS
		AU3_Send(L"{ESC}", 0); Sleep(2000);
		
		//tenta fechar pelo menu
		long sel_menu=AU3_WinMenuSelectItem(L"[CLASS:PCSX2 Main]",L"",L"&File",L"E&xit",L"",L"",L"",L"",L"",L"");
		if(sel_menu){
			Sleep(2000);
		}
		
		//fecha caso esteja aberto
		if(AU3_ProcessExists(L"pcsx2.exe")){						
			Sleep(2000);
			AU3_ProcessClose(L"pcsx2.exe");
			Sleep(2000);
		}

		limpa_memoria();
		allegro_exit();
        return 0;
}
END_OF_MAIN()


//==============================================================================
//seleciona o modo player1 ou player2
//==============================================================================
UCHAR seleciona_modo_inicial(UCHAR num_player){
	DWORD inicio_timer=0, fim_timer=0;
	UCHAR resultado=0;
	
	renderer.update("ESPERANDO MENU");
	inicio_timer=GetTickCount();
	while(true){
		if(espera_dvdinfo(104928, 104928, NULL, 2000)){
			resultado=1;
			break;
		}else{
			if(num_player==1){ENVIA1_S;} //entra como player1
			if(num_player==2){ENVIA2_S;} //entra como player2
		}
		
		//sai se não encontrar em 20 segundos
		fim_timer=GetTickCount();
		if((fim_timer-inicio_timer) > TEMPOAGUARDA){
			PlaySound(".\\soms\\tela_erro.wav", 0, SND_FILENAME | SND_ASYNC);
			renderer.update("MENU NAO ENCONTRADO"); Sleep(5000);
			resultado=0;
			break;
		}
		Sleep(1000);
	} 
	
	//corrige o nivel de dificuldade, eo max_power
	corrige_dificuldade();

	//envia um bolinha para ativar a opção do menu
	if(resultado){
		Sleep(1000);
		if(num_player==1){ENVIA1_B;}
		if(num_player==2){ENVIA2_B;}
		Sleep(1000);
	}

	return resultado;
}

//==============================================================================
//espera o jogo terminar player1 vs cpu
//os estagios são contados começando de 0 até 7 que é o maximo visto até agora
//com isso temos:
//estagios:0,1,2,3,5 que são normais, ou seja 3x3
//estagios:4,6,7 que são diferentes, ou seja 3x1, sendo que o estagio 7 não é sempre
//que aparece, aparentemente vem conforme o desempenho do jogador
//==============================================================================
UCHAR espera_terminar_jogo_p1(void){
	const DWORD PT_EXE_BASE=0x00400000;		//base do executavel
	const DWORD OFFSET_BASE=0x02433700;		//offset 
	const DWORD OF_RO	=0x00000358;		//quantidade de rounds
	const DWORD OF_V1	=0x00000251;		//vitorias player1
	const DWORD OF_V2	=0x00000281;		//vitorias player2
	const DWORD OF_CPU1	=0x00000244;		//lutador1 do player2
	const DWORD OF_CPU2	=0x00000245;		//lutador2 do player2
	const DWORD OF_CPU3	=0x00000246;		//lutador3 do player2
	
	//guarda os endereços corretos
	DWORD PONTEIRO_RO=0;
	DWORD PONTEIRO_V1=0;
	DWORD PONTEIRO_V2=0;
	DWORD PONTEIRO_CPU1=0;
	DWORD PONTEIRO_CPU2=0;
	DWORD PONTEIRO_CPU3=0;
	
	//guarda os resultados
	UCHAR RO=0, V1=0, V2=0, CPU1=0, CPU2=0, CPU3=0;
	
	//outras
	UCHAR gameover=0, retorno=0;
	DWORD fim_timer=0, inicio_timer=0;
	char texto[50];	
	HANDLE processo=NULL;
	bool alterna=false;

	//abre o processo
	processo=abre_processo_memoria();	
	if(!processo){
		renderer.update("ERRO ABRINDO PROCESSO"); Sleep(5000);
		retorno=0xff;
		return retorno;
	}


	//pega a base para os endereços corretos
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_RO, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_V1, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_V2, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_CPU1, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_CPU2, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_CPU3, sizeof(DWORD), 0);

	PRESS2_S=FALSE;
	while(!sair){	//o {sair} é somente para nivel de teste, pois nesse caso não deveria poder sair.
		RO=0; V1=0; V2=0;
		ReadProcessMemory(processo, (LPVOID)(PONTEIRO_RO + OF_RO), &RO, sizeof(UCHAR), 0);	//qt rounds
		ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V1 + OF_V1), &V1, sizeof(UCHAR), 0);	//vitorias p1
		ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V2 + OF_V2), &V2, sizeof(UCHAR), 0);	//vitorias p2
		
		//verifica um pedido de entrada do player2, aqui poderia ser verificado o estagio e caso fosse
		//um estagio igual ou acima de 6 {mestres} não deixaria entrar.
		if((fichas > 0) && (PRESS2_S==TRUE) && (PLAYER2.get_jogando()==false) && (PLAYER1.get_estagio() < 6)){
			desabilita_joy(1); PLAYER1.set_jogando(false);
			pega_ficha(false);
			PlaySound(".\\soms\\versus.wav", 0, SND_FILENAME | SND_ASYNC);
			PRESS2_S=FALSE;
			retorno=2;
			break;
		}

		//aqui é verificado os estágios 1,2,3,4 e depois o 6
		//os outros estágios é apenas 3x1 {especial/mestre}
		if((PLAYER1.get_estagio() <= 3) || (PLAYER1.get_estagio() == 5)){
			//verifica se o player1 ganhou
			if((V1 >= 3) && (V1 > V2)){
				ReadProcessMemory(processo, (LPVOID)(PONTEIRO_CPU1 + OF_CPU1), &CPU1, sizeof(UCHAR), 0);
				ReadProcessMemory(processo, (LPVOID)(PONTEIRO_CPU2 + OF_CPU2), &CPU2, sizeof(UCHAR), 0);
				ReadProcessMemory(processo, (LPVOID)(PONTEIRO_CPU3 + OF_CPU3), &CPU3, sizeof(UCHAR), 0);
				PLAYER1.set_estagio();
				PLAYER1.set_cpu1(CPU1); PLAYER1.set_cpu2(CPU2); PLAYER1.set_cpu3(CPU3); //pega os adverssários

				PLAYER1.set_vitorias();
				sprintf_s(texto, 50, "* VITORIA N\xb0 %d *", PLAYER1.get_vitorias());
				renderer.update(texto);
				PlaySound(".\\soms\\vitoria.wav", 0, SND_FILENAME | SND_ASYNC);
				Sleep(3000);

				//espera para ordenar os chars
				espera_ordenar_chars(1);

				//espera zerar, antes de começar a lêr novamente
				renderer.update("AGUARDANDO...");
				inicio_timer=GetTickCount();
				while(V1 > 0){													
					ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V1 + OF_V1), &V1, sizeof(UCHAR), 0);
					fim_timer=GetTickCount();
					if((fim_timer-inicio_timer) > (TEMPOAGUARDA * 3)){
						renderer.update("ALGO ERRADO 1"); Sleep(5000);
						retorno=255; sair=TRUE;
						break;
					}
					Sleep(1000);
				}				
			}
		}

		//aqui é verificado somente o estágio 5 e 7 e um possivel 8
		//por conter apenas 1 luta contra o CPU, ou seja é um  3 x 1
		if((PLAYER1.get_estagio() == 4) || (PLAYER1.get_estagio() >= 6)){
			if(V1 >= 1){
				PLAYER1.set_estagio();
				PLAYER1.set_vitorias();
				sprintf_s(texto, 50, "* VITORIA N\xb0 %d *", PLAYER1.get_vitorias());
				renderer.update(texto);
				PlaySound(".\\soms\\vitoria.wav", 0, SND_FILENAME | SND_ASYNC);
				Sleep(3000);
				
				//espera ordenar somente no stagio 5, que é depois da primeira luta especial
				//o motivo é por causa do filminho que irá passar entre os outros estagios
				if(PLAYER1.get_estagio() <= 5){
					espera_ordenar_chars(1);
				}
				
				//espera zerar, antes de começar a lêr novamente
				renderer.update("AGUARDANDO...");
				inicio_timer=GetTickCount();
				while(V1 > 0){													
					ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V1 + OF_V1), &V1, sizeof(UCHAR), 0);
					if(V1 == 0){  //se zerou então foi para a proxima luta
						gameover=0;
						break;
					}
					
					//testa uma possivel proxima luta, baseado na ordenação de chars
					//que caso apareça significa que há um estagio a mais.
					if(espera_dvdinfo(259920, 259920, processo, 1000)){
						espera_ordenar_chars(1);
						gameover=0;
						break;
					}

					//provoca a saida encontrando a tela de créditos finais
					if(espera_dvdinfo(1312000, 1329728, processo, 1000)){						
						gameover=1;
						break;
					}
					
					//provoca uma saida caso encontre a tela do memory card
					if(espera_dvdinfo(140944, 140944, processo, 1000)){						
						gameover=1;
						break;
					}
					
					 //se não zerar em 1 minuto, da gameover, é tempo sulficiente
					//para ver o final e assinar o nome
					fim_timer=GetTickCount();
					if((fim_timer-inicio_timer) > (TEMPOAGUARDA * 3)){
						gameover=1;
						break;
					}
					Sleep(1000);
				}

				//sendo a ultima luta, atenção pois pode haver outra
				if((PLAYER1.get_estagio() >= 6) && (gameover==1)){					
					retorno=trata_gameover();
					break;					
				}
			}
		}//vitoria

		//em todos os casos que o cpu ganhar cai aqui, um caso especial e no estagio 8
		//isso porque se perder o jogo não dá continue, e é mostrado o filminho
		if((V2 >= 3) && (V2 > V1) && (PLAYER1.get_estagio() <= 6)){
			retorno=aguarda_assinar_continue(1);
			if(!retorno){
				retorno=trata_gameover();
				break;
			}
			//envia {start1} para continuar
			ENVIA1_S;
			renderer.update("ESCOLHA O SERVICO");
			Sleep(2000);
			inicio_timer=GetTickCount();	
			while(true){
				ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V1 + OF_V1), &V1, sizeof(UCHAR), 0);
				fim_timer=GetTickCount();
				if((fim_timer-inicio_timer) > (TEMPOAGUARDA / 2)){			//se continuou, a proxima tela é opção para facilitar a proxima luta
					if(espera_dvdinfo(1139077, 1139326, processo, 1000)){	//então caso não selecione em 10 segundos é enviado um bolinha
						ENVIA1_B; Sleep(250);								//para selecionar a opção sem serviço
						break;
					}
				}else{
					sprintf_s(texto, 20, "CONTINUE:%02d", (int)(((TEMPOAGUARDA/2) - (fim_timer - inicio_timer))/1000));
					renderer.update(texto);
				}
				if(espera_dvdinfo(1115616, 1115616, processo, 1000)){	//tela char select
					break;
				}
				Sleep(250);
			}
			retorno=1;	//indica um continue do player1
			break;
		}//derrota
		

		//perda especial, isso pode ocorrer somente no estagio 8
		//nesse caso, mesmo perdendo vai direto para o filminho
		if((V2 >= 3) && (V2 > V1) && (PLAYER1.get_estagio() == 7)){
			inicio_timer=GetTickCount();
			while(true){
				//provoca a saida encontrando a tela de créditos finais
				if(espera_dvdinfo(1312000, 1329728, processo, 1000)){					
					break;
				}

				//tempo maximo para ver o filminho final e assinar
				fim_timer=GetTickCount();
				if((fim_timer-inicio_timer) > (TEMPOAGUARDA * 3)){
					break;
				}
				Sleep(250);
			}
			
			retorno=trata_gameover();	//ajusta as variaaveis globais.
			break;						//sai do looping principal
		}


		//empate, é usado os rounds para saber o resultado
		//o código é identico ao de cima, só é usado o round para diferenciar
		if((V1==V2) && (RO > 5)){
			retorno=aguarda_assinar_continue(1);
			if(!retorno){
				retorno=trata_gameover();
				break;
			}
			//envia {start1} para continuar
			ENVIA1_S;
			renderer.update("ESCOLHA O SERVICO");
			Sleep(2000);
			inicio_timer=GetTickCount();	
			while(true){
				ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V1 + OF_V1), &V1, sizeof(UCHAR), 0);
				fim_timer=GetTickCount();
				if((fim_timer-inicio_timer) > (TEMPOAGUARDA / 2)){			//se continuou, a proxima tela é opção para facilitar a proxima luta
					if(espera_dvdinfo(1139077, 1139326, processo, 1000)){	//então caso não selecione em 10 segundos é enviado um bolinha
						ENVIA1_B; Sleep(250);								//para selecionar a opção sem serviço
						break;
					}
				}else{
					sprintf_s(texto, 20, "CONTINUE:%02d", (int)(((TEMPOAGUARDA/2) - (fim_timer - inicio_timer))/1000));
					renderer.update(texto);
				}
				if(espera_dvdinfo(1115616, 1115616, processo, 1000)){	//tela char select
					break;
				}
				Sleep(250);
			}
			retorno=1;	//indica um continue do player1
			break;											
		}//empate
		
		if(alterna){
			sprintf_s(texto, 30,"CREDITOS:%02d", fichas); alterna = !alterna;
		}else{
			sprintf_s(texto, 50, "ST:%d | RO:%d | 1P:%d | 2P:%d", PLAYER1.get_estagio()+1, (RO+1), V1, V2); alterna = !alterna;
		}
		renderer.update(texto);
		Sleep(1000);
	}//while principal
	
	//se for feito o pedido, força a saida
	if(sair){retorno=0xff;}

	//fecha o handler
	fecha_processo_memoria(processo);
	return retorno;
}


//==============================================================================
//espera o jogo terminar player2
//==============================================================================
UCHAR espera_terminar_jogo_p2(void){  
	const DWORD PT_EXE_BASE=0x00400000;		//base do executavel
	const DWORD OFFSET_BASE=0x02433700;		//offset 
	const DWORD OF_RO	=0x00000358;		//quantidade de rounds
	const DWORD OF_V1	=0x00000251;		//vitorias player1
	const DWORD OF_V2	=0x00000281;		//vitorias player2
	const DWORD OF_CPU1	=0x00000274;		//lutador1 do player1
	const DWORD OF_CPU2	=0x00000275;		//lutador2 do player1
	const DWORD OF_CPU3	=0x00000276;		//lutador3 do player1
	
	//guarda os endereços corretos
	DWORD PONTEIRO_RO=0;
	DWORD PONTEIRO_V1=0;
	DWORD PONTEIRO_V2=0;
	DWORD PONTEIRO_CPU1=0;
	DWORD PONTEIRO_CPU2=0;
	DWORD PONTEIRO_CPU3=0;
	
	//guarda os resultados
	UCHAR RO=0, V1=0, V2=0, CPU1=0, CPU2=0, CPU3=0;
	
	//outras
	UCHAR gameover=0, retorno=0;
	DWORD fim_timer=0, inicio_timer=0;
	char texto[50];	
	HANDLE processo=NULL;
	bool alterna=false;

	//abre o processo
	processo=abre_processo_memoria();	
	if(!processo){
		renderer.update("ERRO ABRINDO PROCESSO"); Sleep(5000);
		retorno=0xff;
		return retorno;
	}


	//pega a base para os endereços corretos
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_RO, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_V1, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_V2, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_CPU1, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_CPU2, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_CPU3, sizeof(DWORD), 0);
	
	PRESS1_S=FALSE;
	while(!sair){
		RO=0; V1=0; V2=0;
		ReadProcessMemory(processo, (LPVOID)(PONTEIRO_RO + OF_RO), &RO, sizeof(UCHAR), 0);	//qt rounds
		ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V1 + OF_V1), &V1, sizeof(UCHAR), 0);	//vitorias p1
		ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V2 + OF_V2), &V2, sizeof(UCHAR), 0);	//vitorias p2
		
		//verifica um pedido de entrada do player1		
		if((fichas > 0) && (PRESS1_S==TRUE) && (PLAYER1.get_jogando()==false) && (PLAYER2.get_estagio() < 6)){
			desabilita_joy(2); PLAYER2.set_jogando(false);
			pega_ficha(false);
			PlaySound(".\\soms\\versus.wav", 0, SND_FILENAME | SND_ASYNC);
			PRESS1_S=FALSE;
			retorno=2;
			break;
		}

		//player2 ganhou, aqui é verificado até o estágio 3 e depois o 5
		//entre esse estagios entram dois com apenas um lutador
		if((PLAYER2.get_estagio() <= 3) || (PLAYER2.get_estagio() == 5)){
			if((V2 >= 3) && (V2 > V1)){
				ReadProcessMemory(processo, (LPVOID)(PONTEIRO_CPU1 + OF_CPU1), &CPU1, sizeof(UCHAR), 0);
				ReadProcessMemory(processo, (LPVOID)(PONTEIRO_CPU2 + OF_CPU2), &CPU2, sizeof(UCHAR), 0);
				ReadProcessMemory(processo, (LPVOID)(PONTEIRO_CPU3 + OF_CPU3), &CPU3, sizeof(UCHAR), 0);
				PLAYER2.set_estagio();
				PLAYER2.set_cpu1(CPU1); PLAYER2.set_cpu2(CPU2); PLAYER2.set_cpu3(CPU3); //pega os adverssários

				PLAYER2.set_vitorias();
				sprintf_s(texto, 50, "* VITORIA N\xb0 %d *", PLAYER2.get_vitorias());
				renderer.update(texto);
				PlaySound(".\\soms\\vitoria.wav", 0, SND_FILENAME | SND_ASYNC);
				Sleep(3000);

				//espera para ordenar os chars
				espera_ordenar_chars(2);

				//espera zerar, antes de começar a lêr novamente
				renderer.update("AGUARDANDO...");
				inicio_timer=GetTickCount();
				while(V2 > 0){													
					ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V2 + OF_V2), &V2, sizeof(UCHAR), 0);
					fim_timer=GetTickCount();
					if((fim_timer-inicio_timer) > (TEMPOAGUARDA * 3)){
						renderer.update("ALGO ERRADO 1"); Sleep(5000);
						retorno=255; sair=TRUE;
						break;
					}
					Sleep(1000);
				}				
			}
		}

		//player2 ganhou, aqui é verificado somente o estágio 4 e 6
		//porque eles contém apenas uma luta
		if((PLAYER2.get_estagio() == 4) || (PLAYER2.get_estagio() >= 6)){
			if(V2 >= 1){
				PLAYER2.set_estagio();
				PLAYER2.set_vitorias();
				sprintf_s(texto, 50, "* VITORIA N\xb0 %d *", PLAYER2.get_vitorias());
				renderer.update(texto);
				PlaySound(".\\soms\\vitoria.wav", 0, SND_FILENAME | SND_ASYNC);
				Sleep(3000);
				
				//espera ordenar somente no stagio 5, que é depois da primeira luta especial
				if(PLAYER2.get_estagio() <= 5){
					espera_ordenar_chars(2);
				}
				
				//espera zerar, antes de começar a lêr novamente
				renderer.update("AGUARDANDO...");
				inicio_timer=GetTickCount();
				while(V2 > 0){													
					ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V2 + OF_V2), &V2, sizeof(UCHAR), 0);
					if(V2 == 0){  //se zerou então foi para a proxima luta
						gameover=0;
						break;
					}
					
					//testa uma possivel proxima luta, baseado na ordenação de chars
					//que caso apareça significa que houve um round 7
					if(espera_dvdinfo(259920, 259920, processo, 1000)){
						espera_ordenar_chars(2);
						gameover=0;
						break;
					}

					//provoca a saida encontrando a tela de créditos finais
					if(espera_dvdinfo(1312000, 1329728, processo, 1000)){						
						gameover=1;
						break;
					}
					
					//provoca uma saida caso ache a tela do memory card
					if(espera_dvdinfo(140944, 140944, processo, 1000)){						
						gameover=1;
						break;
					}

					 //se não zera em 1 minuto, da gameover
					fim_timer=GetTickCount();
					if((fim_timer-inicio_timer) > (TEMPOAGUARDA * 3)){
						gameover=1;
						break;
					}
					Sleep(1000);
				}

				//sendo a ultima luta, atenção pois pode haver outra
				if((PLAYER2.get_estagio() >= 6) && (gameover==1)){					
					retorno=trata_gameover();
					break;					
				}
			}
		}//vitoria

		//cpu ganhou, em todos os casos que o cpu ganhar cai aqui
		//com excessão do estágio especial
		if((V1 >= 3) && (V1 > V2) && (PLAYER2.get_estagio() <= 6)){
			retorno=aguarda_assinar_continue(2);
			if(!retorno){
				retorno=trata_gameover();
				break;
			}
			//envia {start2} para continuar
			ENVIA2_S;
			renderer.update("ESCOLHA O SERVICO");
			Sleep(2000);
			inicio_timer=GetTickCount();	
			while(true){
				ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V2 + OF_V2), &V2, sizeof(UCHAR), 0);
				fim_timer=GetTickCount();
				if((fim_timer-inicio_timer) > (TEMPOAGUARDA / 2)){			//se continuou, a proxima tela é opção para facilitar a proxima luta
					if(espera_dvdinfo(1139077, 1139326, processo, 1000)){	//então caso não selecione em 10 segundos é enviado um bolinha
						ENVIA2_B; Sleep(250);								//para selecionar a opção sem serviço
						break;
					}
				}else{
					sprintf_s(texto, 20, "CONTINUE:%02d", (int)(((TEMPOAGUARDA/2) - (fim_timer - inicio_timer))/1000));
					renderer.update(texto);
				}
				if(espera_dvdinfo(1115616, 1115616, processo, 1000)){	//tela char select
					break;
				}
				Sleep(250);
			}
			retorno=1;	//indica um continue do player2
			break;
		}//derrota
		
		//perda especial, isso pode ocorrer somente no estagio 8
		//nesse caso, mesmo perdendo vai direto para o filminho
		if((V1 >= 3) && (V1 > V2) && (PLAYER2.get_estagio() == 7)){
			inicio_timer=GetTickCount();
			while(true){
				//provoca a saida encontrando a tela de créditos finais
				if(espera_dvdinfo(1312000, 1329728, processo, 1000)){					
					break;
				}

				//tempo maximo para ver o filminho final e assinar
				fim_timer=GetTickCount();
				if((fim_timer-inicio_timer) > (TEMPOAGUARDA * 3)){
					break;
				}
				Sleep(250);
			}
			
			retorno=trata_gameover();	//ajusta as variaaveis globais.
			break;						//sai do looping principal
		}

		//empate, é usado os rounds para saber o resultado
		//o código é identico ao de cima, só é usado o round para diferenciar
		if((V2==V1) && (RO > 5)){
			retorno=aguarda_assinar_continue(2);
			if(!retorno){
				retorno=trata_gameover();
				break;
			}
			//envia {start1} para continuar
			ENVIA2_S;
			renderer.update("ESCOLHA O SERVICO");
			Sleep(2000);
			inicio_timer=GetTickCount();	
			while(true){
				ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V2 + OF_V2), &V2, sizeof(UCHAR), 0);
				fim_timer=GetTickCount();
				if((fim_timer-inicio_timer) > (TEMPOAGUARDA / 2)){			//se continuou, a proxima tela é opção para facilitar a proxima luta
					if(espera_dvdinfo(1139077, 1139326, processo, 1000)){	//então caso não selecione em 10 segundos é enviado um bolinha
						ENVIA2_B; Sleep(250);								//para selecionar a opção sem serviço
						break;
					}
				}else{
					sprintf_s(texto, 20, "CONTINUE:%02d", (int)(((TEMPOAGUARDA/2) - (fim_timer - inicio_timer))/1000));
					renderer.update(texto);
				}
				if(espera_dvdinfo(1115616, 1115616, processo, 1000)){	//tela char select
					break;
				}
				Sleep(250);
			}
			retorno=1;	//indica um continue do player2
			break;											
		}//empate
		
		if(alterna){
			sprintf_s(texto, 30,"CREDITOS:%02d", fichas); alterna = !alterna;
		}else{
			sprintf_s(texto, 50, "ST:%d | RO:%d | 1P:%d | 2P:%d", PLAYER2.get_estagio()+1, (RO + 1), V1, V2); alterna = !alterna;
		}
		renderer.update(texto);
		Sleep(1000);
	}//while principal
	
	//se for feito o pedido, força a saida
	if(sair){retorno=0xff;}

	//fecha o handler
	fecha_processo_memoria(processo);
	return retorno;
}


//==============================================================================
//espera pelo termino da luta versus
//retorno {1}=player continua, {2}player2 continua, {0}=empate os dois perdem
//aqui é ajustado todo o estado das variaveis, e chama o slot correto para continuar
//==============================================================================
UCHAR espera_terminar_jogo_p3(void){
	const DWORD PT_EXE_BASE=0x00400000;		//base do executavel
	const DWORD OFFSET_BASE=0x02433700;		//offset 
	const DWORD OF_RO	=0x00000358;		//quantidade de rounds
	const DWORD OF_V1	=0x00000251;		//vitorias player1
	const DWORD OF_V2	=0x00000281;		//vitorias player2
	
	//guarda os endereços corretos
	DWORD PONTEIRO_RO=0;
	DWORD PONTEIRO_V1=0;
	DWORD PONTEIRO_V2=0;
	
	//guarda os resultados
	UCHAR RO=0, V1=0, V2=0;
	
	//outras
	UCHAR retorno=0;
	char texto[50];	
	HANDLE processo=NULL;

	//abre o processo
	processo=abre_processo_memoria();	
	if(!processo){
		renderer.update("ERRO ABRINDO PROCESSO"); Sleep(5000);
		retorno=0xff;
		return retorno;
	}


	//pega a base para os endereços corretos
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_RO, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_V1, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_V2, sizeof(DWORD), 0);
	
	//looping principal
	while(!sair){
		RO=0; V1=0; V2=0;
		ReadProcessMemory(processo, (LPVOID)(PONTEIRO_RO + OF_RO), &RO, sizeof(UCHAR), 0);	//qt rounds
		ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V1 + OF_V1), &V1, sizeof(UCHAR), 0);	//vitorias p1
		ReadProcessMemory(processo, (LPVOID)(PONTEIRO_V2 + OF_V2), &V2, sizeof(UCHAR), 0);	//vitorias p2
		
		//verifica se o P1 ganhou
		if((V1 >= 3) && (V1 > V2)){
			PLAYER1.set_vitorias();
			sprintf_s(texto, 50, "* VITORIA N\xb0 %d *", PLAYER1.get_vitorias());
			renderer.update(texto);
			PlaySound(".\\soms\\vitoria.wav", 0, SND_FILENAME | SND_ASYNC);
			Sleep(3000);
			retorno=1;
			break;
		}

		//verifica se o P2 ganhou
		if((V2 >= 3) && (V2 > V1)){
			PLAYER2.set_vitorias();
			sprintf_s(texto, 50, "* VITORIA N\xb0 %d *", PLAYER2.get_vitorias());
			renderer.update(texto);
			PlaySound(".\\soms\\vitoria.wav", 0, SND_FILENAME | SND_ASYNC);
			Sleep(3000);
			retorno=2;
			break;
		}
		
		//em caso de empate
		if((V1==V2) && (RO > 5)){
			renderer.update("* EMPATE *");
			PlaySound(".\\soms\\empate.wav", 0, SND_FILENAME | SND_ASYNC);
			Sleep(3000);
			retorno=3;
			break;
		}
		
		//mostra as menssagens
		if(fichas > 0){
			sprintf_s(texto, 30,"CREDITOS:%02d", fichas);
		}else{
			sprintf_s(texto, 50, "ST:%d | RO:%d | 1P:%d | 2P:%d", 0, RO, V1, V2);
		}
		renderer.update(texto);
		Sleep(1000);
	}//while principal
	
	//se for feito o pedido, força a saida
	if(sair){retorno=0xff; return retorno;}
	
	//arruma a casa para o player1
	if(retorno==1){
		//passa o estágio mais alto
		if(PLAYER2.get_estagio() > PLAYER1.get_estagio()){
			while(PLAYER1.get_estagio() < PLAYER2.get_estagio()){PLAYER1.set_estagio();}
		}
		desabilita_joy(2);
		PLAYER2.zera_tudo();
		
		//carrega o estagio correspondente
		switch(PLAYER1.get_estagio()){
			case 0:
				desabilita_joy(1); PLAYER1.set_jogando(false);
				carrega_slot_numero(9);	//menu inicial
				Sleep(3000);
				ENVIA1_B; Sleep(1000);
				break;
			case 1:			
				carrega_slot_numero(0);
				Sleep(2000);
				break;
			case 2:
				carrega_slot_numero(1);
				Sleep(2000);
				break;
			case 3:
				carrega_slot_numero(2);
				Sleep(2000);
				break;
			default:
				carrega_slot_numero(3);
				Sleep(2000);
				break;
		}
	}

	//arruma a casa para o player2
	if(retorno==2){
		//passa o estágio mais alto
		if(PLAYER1.get_estagio() > PLAYER2.get_estagio()){
			while(PLAYER2.get_estagio() < PLAYER1.get_estagio()){PLAYER2.set_estagio();}
		}
		
		desabilita_joy(1);
		PLAYER1.zera_tudo();
		
		//carrega o estagio correspondente
		switch(PLAYER2.get_estagio()){
			case 0:
				desabilita_joy(2); PLAYER2.set_jogando(false);
				carrega_slot_numero(9);	//menu inicial
				Sleep(4000);
				ENVIA2_B; Sleep(1000);
				break;
			case 1:			
				carrega_slot_numero(4);
				Sleep(2000);
				break;
			case 2:
				carrega_slot_numero(5);
				Sleep(2000);
				break;
			case 3:
				carrega_slot_numero(6);
				Sleep(2000);
				break;
			default:
				carrega_slot_numero(7);
				Sleep(2000);
				break;
		}
	}

	//empate
	if(retorno==3){
		desabilita_joy(3);
		PLAYER1.zera_tudo();
		PLAYER2.zera_tudo();
		carrega_slot_numero(9);	//menu inicial
		Sleep(4000);
		ENVIA1_A; Sleep(1000);
	}
	
	//altera o nivel e o power, para o valor informado pelo operador
	corrige_dificuldade();

	//fecha o handler
	fecha_processo_memoria(processo);
	return retorno;
}


//==============================================================================
//zera as variaveis dos players e carrega o slot 9 {menu principal}
//voltando para a apresentação do jogo, retorna {0} em caso de sucesso,{255}erro
//==============================================================================
UCHAR trata_gameover(void){
	DWORD fim_timer=0, inicio_timer=0;
	UCHAR slot_resultado=0;
	UCHAR resultado=0;
	int tela=0;

	//desabilita os joysticks
	desabilita_joy(3);
	
	//zera os objetos
	PLAYER1.zera_tudo();
	PLAYER2.zera_tudo();

	//verifica se deseja continuar
	renderer.update("* GAME OVER *");
	PlaySound(".\\soms\\gameover.wav", 0, SND_FILENAME | SND_ASYNC);
	
	//carrega o slot do menu1 para depois voltar para a apresentação
	slot_resultado=carrega_slot_numero(9);
	if(slot_resultado){
		inicio_timer=GetTickCount();	
		while(true){
			tela=retorna_tela_atual();									//aqui deve ser usado esse metodo, por que ao carregar um slot
			if((tela >= TELA_MENU1_MIN) && (tela <= TELA_MENU1_MAX)){	//as menssagens mudam em dvdinfo
				Sleep(3000);
				ENVIA1_A; Sleep(250); //volta para apresentação
				resultado=0;
				break;
			}

			//se não encontrar a tela no tempo
			fim_timer=GetTickCount();
			if((fim_timer-inicio_timer) > (TEMPOAGUARDA*2)){
				renderer.update("ERRO ESPERA MENU"); Sleep(5000);
				resultado=0xff;
				break;
			}
			Sleep(1000);
		}
	}else{
		resultado=0xff;
	}

	return resultado;
}


//==============================================================================
//verifica a assinatura e o continue
//==============================================================================
UCHAR aguarda_assinar_continue(UCHAR num_player){
	DWORD fim_timer=0, inicio_timer=0;
	char contador[20];
	UCHAR resultado=0;
	
	//o jogador perdendo então pode ser mostrado a tela
	//de assinatura, ou então o continue
	renderer.update("AGUARDANDO CONTINUE");
	while(true){
		if(espera_dvdinfo(1138390, 1139050, NULL, 2000)){
			break;
		}
		Sleep(1000);
	}

	//verifica se deseja continuar
	inicio_timer=GetTickCount();
	PRESS1_S=FALSE; PRESS2_S=FALSE;
	while (TRUE){
		if(num_player==1){
			if(fichas > 0 && PRESS1_S==TRUE){
				pega_ficha(false);
				resultado=1;
				PRESS1_S=FALSE;
				break;
			}
		}else if(num_player==2){
			if(fichas > 0 && PRESS2_S==TRUE){
				pega_ficha(false);
				resultado=1;
				PRESS2_S=FALSE;
				break;
			}
		}
		
		//verifica se ainda está na tela de continue
		if(!espera_dvdinfo(1138390, 1139050, NULL, 1000)){
			resultado=0;
			break;
		}

		fim_timer=GetTickCount();
		if((fim_timer-inicio_timer) > TEMPOAGUARDA){			
			resultado=0;
			break;
		}else{
			sprintf_s(contador, 20, "CONTINUE:%02d", (int)((TEMPOAGUARDA - (fim_timer - inicio_timer))/1000));
			renderer.update(contador);
		}
		Sleep(1000);
	}

	return resultado;
}


//==============================================================================
//carrega o slot informado
//==============================================================================
UCHAR carrega_slot_numero(char num_slot){
	DWORD fim_timer=0, inicio_timer=0;
	UCHAR resultado=0;
	int slot_atual=0;
	
	renderer.update("PROCURANDO SLOT");
	inicio_timer=GetTickCount();
	while(TRUE){
		slot_atual=retorna_slot_atual();
		if(slot_atual != num_slot){
			AU3_Send(L"{F2}", 0); Sleep(100); //{F2} muda o slot
		}else{
			resultado=1;
			break;
		}

		fim_timer=GetTickCount();
		if((fim_timer-inicio_timer) > TEMPOAGUARDA){
			renderer.update("SLOT NAO ENCONTRADO"); Sleep(5000);
			resultado=0;
			break;
		}
		Sleep(250);
	}

	if(resultado){
		AU3_Send(L"{F3}", 0);					//{F3} carrega o slot
		if(num_slot==8){						//envia mais um para corrigir
			Sleep(4000);
			ENVIA1_B;	Sleep(250);
		}
	}

	return resultado;
}

//==============================================================================
//espera pela tela de inicio de luta
//==============================================================================
UCHAR espera_tela_inicio_luta(void){
	DWORD fim_timer=0, inicio_timer=0;
	UCHAR resultado;

	renderer.update("AGUARDANDO O INICIO");
	inicio_timer=GetTickCount();
	while (TRUE){

		//verifica se já selecionou os chars
		if(!espera_dvdinfo(259920, 259920, NULL, 3000)){
			resultado=1;
			break;
		}
		
		fim_timer=GetTickCount();
		if((fim_timer-inicio_timer) > TEMPOAGUARDA){
			PlaySound(".\\soms\\tela_erro.wav", 0, SND_FILENAME | SND_ASYNC);
			renderer.update("ERRO TELA INICIO JOGO"); Sleep(5000);
			resultado=0;
			break;
		}
		Sleep(1000);
	}
	return resultado;
}

//==============================================================================
//espera a ordenação dos chars do player1
//==============================================================================
UCHAR espera_ordenar_chars(UCHAR num_player){
	#define TEMPO_ORDENA_CHAR 20000
	DWORD fim_timer=0, inicio_timer=0;
	char contador[20];
	UCHAR resultado=0;

	//primeiro aguarda até que esteja na tela de ordenação
	inicio_timer=GetTickCount();
	while (TRUE){		
		if(espera_dvdinfo(259920, 259920, NULL, 3000)){	//tela ordenar
			break;
		}

		//sai se ficar esperando por mais de 1 minuto
		fim_timer=GetTickCount();
		if((fim_timer-inicio_timer) > (TEMPOAGUARDA*3)){
			break;
		}
		Sleep(1000);
	}

	//gera os char adverssários, se não for o versus
/*	switch(num_player){
		case 1:
			if((PLAYER1.get_estagio() <=3) || (PLAYER1.get_estagio() == 5)){
				gera_random_char(2);
			}
			break;
		case 2:
			if((PLAYER2.get_estagio() <=3) || (PLAYER2.get_estagio() == 5)){
				gera_random_char(1);
			}
			break;
	}
*/

	//espera selecionar a ordem dos chars	
	renderer.update("ORDENAR OS LUTADORES");
	inicio_timer=GetTickCount();
	while (TRUE){
		//se não estiver mais na tela de ordenar
		if(!espera_dvdinfo(259920, 259920, NULL, 3000)){
			resultado=1;
			break;
		}

		fim_timer=GetTickCount();
		if((fim_timer-inicio_timer) > TEMPO_ORDENA_CHAR){
			for(int a=0; a <= 2 ; a++){
				if(num_player==1 || num_player==3){ENVIA1_X; Sleep(250);}
				if(num_player==1 || num_player==3){ENVIA1_Y; Sleep(250);}
				if(num_player==1 || num_player==3){ENVIA1_A; Sleep(250);}
				if(num_player==2 || num_player==3){ENVIA2_X; Sleep(250);}
				if(num_player==2 || num_player==3){ENVIA2_Y; Sleep(250);}
				if(num_player==2 || num_player==3){ENVIA2_A; Sleep(250);}
			}
			resultado=1;
			break;
		}else{
			sprintf_s(contador, 20, "TEMPO:%02d", (int)((TEMPO_ORDENA_CHAR - (fim_timer - inicio_timer))/1000));
			renderer.update(contador);
		}
		Sleep(1000);
	}
	//mostras os créditos para tirar a menssagem de tempo
	sprintf_s(contador, 20, "CREDITOS:%02d", fichas);
	renderer.update(contador);

	return resultado;
}


//==============================================================================
//espera o player1 selecionar o char
//==============================================================================
UCHAR espera_selecionar_player1(void){	
	DWORD fim_timer=0, inicio_timer=0;
	char contador[20];
	UCHAR resultado=0;
		
	//agora o joystick é habilitado para que ele possa escolher
	if(PLAYER1.get_jogando()==false){
		habilita_joy(1);
		PLAYER1.set_jogando(true);
	}

	//espera o player1 selecionar os chars
	renderer.update("SELECIONE OS CHARS");
	PlaySound(".\\soms\\seleciona_lutadores.wav", 0, SND_FILENAME | SND_ASYNC);

	PRESS2_S=FALSE;
	inicio_timer=GetTickCount();
	while (TRUE){
		if((fichas > 0) && (PRESS2_S==TRUE) && (PLAYER2.get_jogando()==false)){	//pedido de entrada do player2
			desabilita_joy(1); PLAYER1.set_jogando(false);
			pega_ficha(false);
			PlaySound(".\\soms\\versus.wav", 0, SND_FILENAME | SND_ASYNC);
			PRESS2_S=FALSE;
			resultado=2;
			break;
		}
		
		//verifica se já selecionou os chars
		if(espera_dvdinfo(259920, 259920, NULL, 1000)){
			resultado=1;
			break;
		}

		//se não selecionar dentro do tempo então o programa seleciona automaticamente
		fim_timer=GetTickCount();
		if((fim_timer-inicio_timer) > TEMPO_SELECIONA_CHAR){
			for(int a=0; a <= 3 ; a++){
				ENVIA1_B; Sleep(250);
			}
			resultado=1;
			break;
		}else{
			sprintf_s(contador, 20, "TEMPO:%02d", (int)((TEMPO_SELECIONA_CHAR - (fim_timer - inicio_timer))/1000));
			renderer.update(contador);
		}
		Sleep(250);
	}
	if(resultado==2){return resultado;} //caso o player2 queira entrar

	//gera os char adverssários
	if((PLAYER1.get_estagio() <=3) || (PLAYER1.get_estagio() == 5)){
		gera_random_char(2);
	}

	//agora espera até que os chars sejam selecionados
	resultado=espera_ordenar_chars(1);
	
	return resultado;
}


//==============================================================================
//espera o player2 selecionar o char
//==============================================================================
UCHAR espera_selecionar_player2(void){
	DWORD fim_timer=0, inicio_timer=0;
	char contador[20];
	UCHAR resultado=0;
		
	//agora o joystick é habilitado para que ele possa escolher
	if(PLAYER2.get_jogando()==false){
		habilita_joy(2);
		PLAYER2.set_jogando(true);
	}

	//espera o player2 selecionar os chars
	renderer.update("SELECIONE OS CHARS");
	PlaySound(".\\soms\\seleciona_lutadores.wav", 0, SND_FILENAME | SND_ASYNC);

	PRESS1_S=FALSE;
	inicio_timer=GetTickCount();
	while (TRUE){
		if((fichas > 0) && (PRESS1_S==TRUE) && (PLAYER1.get_jogando()==false)){	//pedido de entrada do player1
			desabilita_joy(2); PLAYER1.set_jogando(false);
			pega_ficha(false);
			PlaySound(".\\soms\\versus.wav", 0, SND_FILENAME | SND_ASYNC);
			PRESS1_S=FALSE;
			resultado=2;
			break;
		}
		
		//verifica se já selecionou os chars
		if(espera_dvdinfo(259920, 259920, NULL, 1000)){
			resultado=1;
			break;
		}

		//se não selecionar dentro do tempo então o programa seleciona automaticamente
		fim_timer=GetTickCount();
		if((fim_timer-inicio_timer) > TEMPO_SELECIONA_CHAR){
			for(int a=0; a <= 3 ; a++){
				ENVIA2_B; Sleep(250);
			}
			resultado=1;
			break;
		}else{
			sprintf_s(contador, 20, "TEMPO:%02d", (int)((TEMPO_SELECIONA_CHAR - (fim_timer - inicio_timer))/1000));
			renderer.update(contador);
		}
		Sleep(250);
	}
	if(resultado==2){return resultado;} //caso o player1 queira entrar

	//gera os char adverssários
	if((PLAYER2.get_estagio() <=3) || (PLAYER2.get_estagio() == 5)){
		gera_random_char(1);
	}

	//agora espera até que os chars sejam selecionados
	resultado=espera_ordenar_chars(2);
	
	return resultado;	
}


//==============================================================================
//espera o player1 eo player2 selecionar o char, retorna{1}=ok, {0}=erro
//==============================================================================
UCHAR espera_selecionar_player3(void){
	DWORD fim_timer=0, inicio_timer=0;
	char contador[20];
	UCHAR resultado=0;
	
	//primeiro carrega o slot8 que é o versus
	if(!carrega_slot_numero(8)){
		renderer.update("ERRO SLOT VERSUS"); Sleep(5000);
		return resultado;
	}else{
		Sleep(3000);
	}

	//espera o player1 selecionar os chars
	renderer.update("SELECIONE OS CHARS");
	PlaySound(".\\soms\\seleciona_lutadores.wav", 0, SND_FILENAME | SND_ASYNC);
	Sleep(2000);

	//agora o joystick é habilitado para que ele possa escolher
	habilita_joy(3);
	PLAYER1.set_jogando(true);
	PLAYER2.set_jogando(true);

	inicio_timer=GetTickCount();
	while (TRUE){		
		//verifica se já selecionou os chars
		if(espera_dvdinfo(259920, 259920, NULL, 1000)){
			break;
		}

		//se não selecionar dentro do tempo então o programa seleciona automaticamente
		fim_timer=GetTickCount();
		if((fim_timer-inicio_timer) > TEMPO_SELECIONA_CHAR){
			for(int a=0; a <= 3 ; a++){
				ENVIA1_B; Sleep(250);
				ENVIA2_B; Sleep(250);
			}
			resultado=1;
			break;
		}else{
			sprintf_s(contador, 20, "TEMPO:%02d", (int)((TEMPO_SELECIONA_CHAR - (fim_timer - inicio_timer))/1000));
			renderer.update(contador);
		}
		Sleep(250);
	}

	//agora espera até que os chars sejam selecionados
	resultado=espera_ordenar_chars(3);
	
	return resultado;	
}

//==============================================================================
//gera os adverssários do player
//==============================================================================
void gera_random_char(UCHAR num){
	const DWORD PT_EXE_BASE=0x00400000;		//base do executavel
	const DWORD OFFSET_BASE=0x02433700;		//offset 

	const DWORD OF_P1_CPU1	=0x00000274;		//lutador1 do player1
	const DWORD OF_P1_CPU2	=0x00000275;		//lutador2 do player1
	const DWORD OF_P1_CPU3	=0x00000276;		//lutador3 do player1

	const DWORD OF_P2_CPU1	=0x00000244;		//lutador1 do player2
	const DWORD OF_P2_CPU2	=0x00000245;		//lutador2 do player2
	const DWORD OF_P2_CPU3	=0x00000246;		//lutador3 do player2

	DWORD PONTEIRO_P1_CPU1=0, PONTEIRO_P1_CPU2=0, PONTEIRO_P1_CPU3=0;
	DWORD PONTEIRO_P2_CPU1=0, PONTEIRO_P2_CPU2=0, PONTEIRO_P2_CPU3=0;

	HANDLE processo=NULL;
	UCHAR numero=0;
	UCHAR lutador[3];

	//abre o processo
	processo=abre_processo_memoria_write();	
	if(!processo){
		return;
	}
	
	srand (time(NULL));				//gera um numero seed
	for(UCHAR a=0;a<3;a++){
		lutador[a]=255;				//inicializa o array
		numero=(UCHAR)(rand()%50);	//gera o numero de 0 até 50
		while((numero == lutador[0]) || (numero == lutador[1])){
			numero=(UCHAR)(rand()%50);
			Sleep(100);
		}		
		lutador[a]=numero;
	}

	if(num==1){		
		ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_P1_CPU1, sizeof(DWORD), 0);
		ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_P1_CPU2, sizeof(DWORD), 0);
		ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_P1_CPU3, sizeof(DWORD), 0);

		WriteProcessMemory(processo, (LPVOID)(PONTEIRO_P1_CPU1 + OF_P1_CPU1), &lutador[0], sizeof(UCHAR), 0);
		WriteProcessMemory(processo, (LPVOID)(PONTEIRO_P1_CPU2 + OF_P1_CPU2), &lutador[1], sizeof(UCHAR), 0);
		WriteProcessMemory(processo, (LPVOID)(PONTEIRO_P1_CPU3 + OF_P1_CPU3), &lutador[2], sizeof(UCHAR), 0);
	}else{
		ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_P2_CPU1, sizeof(DWORD), 0);
		ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_P2_CPU2, sizeof(DWORD), 0);
		ReadProcessMemory(processo, (LPVOID)(PT_EXE_BASE + OFFSET_BASE), &PONTEIRO_P2_CPU3, sizeof(DWORD), 0);

		WriteProcessMemory(processo, (LPVOID)(PONTEIRO_P2_CPU1 + OF_P2_CPU1), &lutador[0], sizeof(UCHAR), 0);
		WriteProcessMemory(processo, (LPVOID)(PONTEIRO_P2_CPU2 + OF_P2_CPU2), &lutador[1], sizeof(UCHAR), 0);
		WriteProcessMemory(processo, (LPVOID)(PONTEIRO_P2_CPU3 + OF_P2_CPU3), &lutador[2], sizeof(UCHAR), 0);
	}

	fecha_processo_memoria(processo);
}


//==============================================================================
//	função callback para os joysticks
//==============================================================================
void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime){
	//verifica o estado do joystick 1, caso esteja conectado
	if (joyGetPos(JOYSTICKID_1, &joy1) == JOYERR_NOERROR){
        //botão coin
        if ((joy1.wButtons & BUTTON5) && (!FLAG1_C)){FLAG1_C = TRUE;}
        if ((!(joy1.wButtons & BUTTON5)) && (FLAG1_C)){FLAG1_C = FALSE; pega_ficha(TRUE);}

		//botão Start1
		if ((joy1.wButtons & BUTTON7) && (FLAG1_S == FALSE)){FLAG1_S = TRUE;}
		if ((!(joy1.wButtons & BUTTON7)) && (FLAG1_S == TRUE)){FLAG1_S = FALSE; PRESS1_S = TRUE;}

		//botão {A} X
		if ((joy1.wButtons & BUTTON3) && (FLAG1_A == FALSE)){FLAG1_A = TRUE;}
		if ((!(joy1.wButtons & BUTTON3)) && (FLAG1_A == TRUE)){FLAG1_A = FALSE; PRESS1_A = TRUE;}

		//botão {B} bolinha
		if ((joy1.wButtons & BUTTON2) && (FLAG1_B == FALSE)){FLAG1_B = TRUE;}
		if ((!(joy1.wButtons & BUTTON2)) && (FLAG1_B == TRUE)){FLAG1_B = FALSE; PRESS1_B = TRUE;}

		//botão {X} quadrado
		if ((joy1.wButtons & BUTTON4) && (FLAG1_X == FALSE)){FLAG1_X = TRUE;}
		if ((!(joy1.wButtons & BUTTON4)) && (FLAG1_X == TRUE)){FLAG1_X = FALSE; PRESS1_X = TRUE;}

		//botão {Y} triangulo
		if ((joy1.wButtons & BUTTON1) && (FLAG1_Y == FALSE)){FLAG1_Y = TRUE;}
		if ((!(joy1.wButtons & BUTTON1)) && (FLAG1_Y == TRUE)){FLAG1_Y = FALSE; PRESS1_Y = TRUE;}

		//botão sair
		if ((joy1.wButtons & BT_SAIR) && (FLAG1_Z == FALSE)){FLAG1_Z = TRUE;}
		if ((!(joy1.wButtons & BT_SAIR)) && (FLAG1_Z == TRUE)){FLAG1_Z = FALSE;	sair = TRUE;}

    }//if primeiro controle

	//verifica o estado do joystick 2, caso esteja conectado
	if (joyGetPos(JOYSTICKID_2, &joy2) == JOYERR_NOERROR){
        //botão coin
        if ((joy2.wButtons & BUTTON5) && (!FLAG2_C)){FLAG2_C = TRUE;}
        if ((!(joy2.wButtons & BUTTON5)) && (FLAG2_C)){FLAG2_C = FALSE; pega_ficha(TRUE);}

		//botão Start2
		if ((joy2.wButtons & BUTTON7) && (FLAG2_S == FALSE)){FLAG2_S = TRUE;}
		if ((!(joy2.wButtons & BUTTON7)) && (FLAG2_S == TRUE)){FLAG2_S = FALSE; PRESS2_S = TRUE;}

		//botão {A} X
		if ((joy2.wButtons & BUTTON3) && (FLAG2_A == FALSE)){FLAG2_A = TRUE;}
		if ((!(joy2.wButtons & BUTTON3)) && (FLAG2_A == TRUE)){FLAG2_A = FALSE; PRESS2_A = TRUE;}

		//botão {B} bolinha
		if ((joy2.wButtons & BUTTON2) && (FLAG2_B == FALSE)){FLAG2_B = TRUE;}
		if ((!(joy2.wButtons & BUTTON2)) && (FLAG2_B == TRUE)){FLAG2_B = FALSE; PRESS2_B = TRUE;}

		//botão {X} quadrado
		if ((joy2.wButtons & BUTTON4) && (FLAG2_X == FALSE)){FLAG2_X = TRUE;}
		if ((!(joy2.wButtons & BUTTON4)) && (FLAG2_X == TRUE)){FLAG2_X = FALSE; PRESS2_X = TRUE;}

		//botão {Y} triangulo
		if ((joy2.wButtons & BUTTON1) && (FLAG2_Y == FALSE)){FLAG2_Y = TRUE;}
		if ((!(joy2.wButtons & BUTTON1)) && (FLAG2_Y == TRUE)){FLAG2_Y = FALSE; PRESS2_Y = TRUE;}
		
		//botão sair
		if ((joy2.wButtons & BT_SAIR) && (FLAG1_Z == FALSE)){FLAG1_Z = TRUE;}
		if ((!(joy2.wButtons & BT_SAIR)) && (FLAG1_Z == TRUE)){FLAG1_Z = FALSE;	sair = TRUE;}
    }//if segundo controle
}

//====================================================================================
//função responssavel por armazenar e retirar as fichas
//====================================================================================
_inline void pega_ficha(BOOL tira_poe){
	char buffer_creditos[20];
	if (tira_poe == TRUE){
		fichas+=(1*CREDITOS_POR_FICHA);
		PlaySound(".\\soms\\coin.wav", 0, SND_FILENAME | SND_ASYNC);
	}else{
		fichas-=1;
		if (fichas < 0){fichas = 0;}
    }
	
	sprintf_s(buffer_creditos, MAX_BUFF_MENSA, "CREDITOS:%02d", fichas);
	renderer.update(buffer_creditos);
}


//====================================================================================
//pega os dados de configurações dos joysticks e botão de saida
//====================================================================================
BOOL configura_joysticks(void){	
    int nc = 0;
    char buffer_joy[MAX_BUFF_INI];
    
    //pega a identificação do primeiro joy
    nc = GetPrivateProfileString("joystick", "num_joy1", "0", buffer_joy, MAX_BUFF_INI, CONFIG_INI);	
	if(nc != 0){
        JOYSTICKID_1 = atoi(buffer_joy);
	}else{
		JOYSTICKID_1=0;
	}
	//agora testa para ver se está correto
	if (joyGetPos(JOYSTICKID_1, &joy1) != JOYERR_NOERROR){
		allegro_message("O joystick numero [%d], informado nao e valido.\nCorrija o parametro no arquivo (%s)\npara o jogo trabalhar corretamente.", JOYSTICKID_1, CONFIG_INI);
		return FALSE;
	}

    //pega a identificação do segundo joy
    nc = GetPrivateProfileString("joystick", "num_joy2", "1", buffer_joy, MAX_BUFF_INI, CONFIG_INI);	
	if(nc != 0){
        JOYSTICKID_2 = atoi(buffer_joy);
	}else{
		JOYSTICKID_2=1;
	}
	//agora testa para ver se está correto
	if (joyGetPos(JOYSTICKID_2, &joy2) != JOYERR_NOERROR){
		allegro_message("O joystick numero [%d], informado nao e valido.\nCorrija o parametro no arquivo (%s)\npara o jogo trabalhar corretamente.", JOYSTICKID_2, CONFIG_INI);
		return FALSE;
	}
	
    //pega a identificação do joy que contem o botão de saida
    nc = GetPrivateProfileString("joystick", "bt_sair", "9", buffer_joy, MAX_BUFF_INI, CONFIG_INI);
	switch(atoi(buffer_joy)){
		case 1: BT_SAIR=BUTTON1; break;
		case 2: BT_SAIR=BUTTON2; break;
		case 3: BT_SAIR=BUTTON3; break;
		case 4: BT_SAIR=BUTTON4; break;
		case 5: BT_SAIR=BUTTON5; break;
		case 6: BT_SAIR=BUTTON6; break;
		case 7: BT_SAIR=BUTTON7; break;
		case 8: BT_SAIR=BUTTON8; break;
		case 9: BT_SAIR=BUTTON9; break;
		case 10: BT_SAIR=BUTTON10; break;
		case 11: BT_SAIR=BUTTON11; break;
		case 12: BT_SAIR=BUTTON12; break;
		case 13: BT_SAIR=BUTTON13; break;
		case 14: BT_SAIR=BUTTON14; break;
		case 15: BT_SAIR=BUTTON15; break;
		default:
			BT_SAIR=BUTTON9;
    }    

    return TRUE;
}

//====================================================================================
// carrega as configurações do arquivo ".ini"
//====================================================================================
BOOL carrega_configuracao(void){    
    char buffer_cred[MAX_BUFF_INI];
    int nc = 0;
	
	//pega o tempo de espera para selecionar um char
	nc = GetPrivateProfileString("config", "tempo_seleciona_char", "30", buffer_cred, MAX_BUFF_INI, CONFIG_INI);
	if (nc != 0){
        TEMPO_SELECIONA_CHAR =(DWORD)(atoi(buffer_cred) * 1000);
	}else{
		TEMPO_SELECIONA_CHAR =(DWORD)(30 * 1000);
	}

	//pega o nivel de dificuldade valores válidos 1 até 6
	nc = GetPrivateProfileString("config", "dificuldade", "3", buffer_cred, MAX_BUFF_INI, CONFIG_INI);
	if (nc != 0){
        NIVEL_DIFICULDADE = (UCHAR)atoi(buffer_cred);
	}
	if((NIVEL_DIFICULDADE < 1) || (NIVEL_DIFICULDADE > 6)){
		NIVEL_DIFICULDADE = 3;
	}

	//pega o valor do MAX POWER 0=normal, 1 = cheio
	nc = GetPrivateProfileString("config", "max_power", "0", buffer_cred, MAX_BUFF_INI, CONFIG_INI);
	if (nc != 0){
        MAX_POWER = (UCHAR)atoi(buffer_cred);
	}
	if((MAX_POWER < 0) || (MAX_POWER > 1)){
		MAX_POWER = 0;
	}

	//pega o valor do volume das musicas de fundo 0=minimo, 7=maximo
	nc = GetPrivateProfileString("config", "volume_musicas", "3", buffer_cred, MAX_BUFF_INI, CONFIG_INI);
	if (nc != 0){
        MAX_MUSICAS = (UCHAR)atoi(buffer_cred);
	}
	if((MAX_MUSICAS < 0) || (MAX_MUSICAS > 7)){
		MAX_MUSICAS = 3;
	}

	//pega o valor do volume das vozes de fundo 0=minimo, 7=maximo
	nc = GetPrivateProfileString("config", "volume_vozes", "6", buffer_cred, MAX_BUFF_INI, CONFIG_INI);
	if (nc != 0){
        MAX_VOZES = (UCHAR)atoi(buffer_cred);
	}
	if((MAX_VOZES < 0) || (MAX_VOZES > 7)){
		MAX_VOZES = 6;
	}

	//pega o multiplicador de creditos
	nc = GetPrivateProfileString("config", "creditos_por_ficha", "1", buffer_cred, 100, CONFIG_INI);
	if (nc != 0){
        CREDITOS_POR_FICHA = atoi(buffer_cred);
	}else{
		CREDITOS_POR_FICHA = 1;
	}
	if(CREDITOS_POR_FICHA < 1){CREDITOS_POR_FICHA=1;}	//garante um valor válido

	//pega a amplitude do volume em jogo
	nc = GetPrivateProfileString("config", "volume_jogo", "24", buffer_cred, 100, CONFIG_INI);
	if (nc != 0){
        VOLUME_JOGO = atoi(buffer_cred);
	}
	if((VOLUME_JOGO < 0) || (VOLUME_JOGO > 24)){
		VOLUME_JOGO=24;
	}

	//pega a amplitude do volume em demo
	nc = GetPrivateProfileString("config", "volume_demo", "24", buffer_cred, 100, CONFIG_INI);
	if (nc != 0){
        VOLUME_DEMO = atoi(buffer_cred);
	}
	if((VOLUME_DEMO < 0) || (VOLUME_DEMO > 24)){
		VOLUME_DEMO = 24;
	}

    //pega a posição onde o texto créditos deve ficar
    nc = GetPrivateProfileString("textos", "credito_x", "200", buffer_cred, MAX_BUFF_INI, CONFIG_INI);
	if (nc != 0){
        credito_x = atoi(buffer_cred);
    }
    nc = GetPrivateProfileString("textos", "credito_y", "400", buffer_cred, MAX_BUFF_INI, CONFIG_INI);
	if (nc != 0){
        credito_y = atoi(buffer_cred);
    }
    nc = GetPrivateProfileString("textos", "credito_t", "24", buffer_cred, MAX_BUFF_INI, CONFIG_INI);
	if (nc != 0){
        credito_t = atoi(buffer_cred);
    }
    nc = GetPrivateProfileString("textos", "credito_o", "340", buffer_cred, MAX_BUFF_INI, CONFIG_INI);
	if (nc != 0){
        credito_o = atoi(buffer_cred);
    }
	
	//cor da menssagem de créditos
    nc = GetPrivateProfileString("textos", "credito_cor", "1", buffer_cred, MAX_BUFF_INI, CONFIG_INI);
	if (nc != 0){
        switch(atoi(buffer_cred)){
            case 1:
                cor_credito = VERMELHO;
                break;
            case 2:
                cor_credito = VERDE;
                break;
            case 3:
                cor_credito = AZUL;
                break;
            case 4:
                cor_credito = BRANCO;
                break;
            case 5:
                cor_credito = PRETO;
                break;
            default:
                cor_credito = BRANCO;
                break;
        }		
    }else{
		cor_credito = BRANCO;
	}

	
	//inicia o overlay/mostra o overlay
	renderer.Ajusta(credito_x, credito_y, credito_o, credito_t);
	renderer.Reset();
	renderer.ShowOverlay();
	renderer.update("CARREGANDO AGUARDE...");
	if(renderer.get_versao() > 5){
		renderer.update("NAO FUNCIONA EM WIN7");
		Sleep(5000);
		return FALSE;
	}

	//para garantir é setado o dvdprint
	if(!AU3_IniWrite(L".\\inis\\pcsx2.ini", L"Console", L"CdvdVerbose", L"enabled")){
		allegro_message("Erro: O arquivo pcsx2.ini, nao esta no local correto");
		return false;
	}

	if(!inicia_jogo()){
		return FALSE;
	}
    Sleep(2000);
	return TRUE;
}


//====================================================================================
//aguarda o deposito de uma ficha e o start, a função será usada assim que
//programa iniciar ou depois de um game over, quando retornar para a apresentação
//====================================================================================
UCHAR aguarda_deposito_ficha(void){
	#define PISCAPISCA 2000
	DWORD inicio_timer=0, fim_timer=0;
	BOOL mostra = FALSE;
	UCHAR retorno=0;
	
	//fica em loop até que uma ficha seja depositada
	inicio_timer=GetTickCount();
	PRESS1_S=FALSE; PRESS2_S=FALSE;
	while(TRUE){
		//caso queira entrar no player1
		if ((fichas > 0) && (PRESS1_S == TRUE)){			
			pega_ficha(FALSE);	//retira 1 crédito
			PRESS1_S = FALSE;
			retorno=1;
			break;
		}else{
			if(PRESS1_S==TRUE){PRESS1_S=FALSE;}
		}
		
		//caso queira entrar no player2
		if ((fichas > 0) && (PRESS2_S == TRUE)){			
			pega_ficha(FALSE);	//retira 1 crédito
			PRESS2_S = FALSE;
			retorno=2;
			break;
		}else{
			if(PRESS2_S==TRUE){PRESS2_S=FALSE;}
		}

		//mostra a menssagem de {insert coin/recorde} ou press start
		fim_timer=GetTickCount();
		if((fim_timer-inicio_timer) > PISCAPISCA){			
			if (fichas > 0){
				renderer.update("PRESS START");				
			}else{
				if(mostra){
					renderer.update("INSERT COIN");
					mostra = !mostra;
				}else{
					renderer.update("CREDITOS:00");
					mostra = !mostra;
				}
			}
			inicio_timer=GetTickCount();
		}		

		//serve para nivel de teste
		if(sair == TRUE){
			retorno=0;
			break;
		}                		
        Sleep(100);
	}
	return retorno;
}

//====================================================================================
//pega o centro do joystick
//====================================================================================
void GetJoyMin( int joy){
	JOYCAPS jc;
	JOYINFO JI1,JI2;
	int centro=0;

	if(joyGetDevCapsA(joy, &jc, sizeof(jc))!= JOYERR_NOERROR){ 
      allegro_message("Erro na calibracao do joy 1");
	  return;
	}
	joyGetPos(joy, &joy1);
	
	centro=joy1.wXpos;

	JI1.wXpos = jc.wXmin;
	JI1.wYpos = jc.wYmin;    
	JI1.wButtons = jc.wNumButtons;

	JI2.wXpos = jc.wXmax;
	JI2.wYpos = jc.wYmax;    
    JI2.wButtons = jc.wNumButtons;

	allegro_message("Xmin=%d, Xmax=%d\n Ymin=%d, Ymax=%d\n BOTOES=%d, CENTRO=%d",  JI1.wXpos, JI2.wXpos, JI1.wYpos, JI2.wYpos, JI1.wButtons, centro);
}


//====================================================================================
//reinicia o jogo para entrar na apresentação
//====================================================================================
BOOL inicia_jogo(void){
	DWORD inicio_timer=0, fim_timer=0;
	//move o ponteiro do mouse
	int width=0, height=0;
	if (get_desktop_resolution(&width, &height) == 0){     	
		AU3_MouseMove(width+100,height+100, 0);
	}

	//testa se o programa esta na pasta corrente
	if (!exists("pcsx2.exe")){
		allegro_message("Erro: Esse programa deve ficar na mesma pasta do pcsx2.exe\n");
		return FALSE;
    }
	
	//chama o loader e aguarda pela janela
	ShellExecute(NULL, "open", "pcsx2.exe", "", ".\\", SW_SHOWNORMAL);
	Sleep(3000);
	if(!AU3_ProcessExists(L"pcsx2.exe")){
		allegro_message("Nao foi possivel abrir o emulador");
		return FALSE;
	}
	
	//seleciona o menu Run CDVD
	long sel_menu=AU3_WinMenuSelectItem(L"[CLASS:PCSX2 Main]",L"",L"&File",L"&Run CD/DVD",L"",L"",L"",L"",L"",L"");
	if(!sel_menu){
		allegro_message("ERRO: ao tentar ativar o menu Run CDVD");
		return FALSE;
	}

	//espera pela janela do GS
	if(!AU3_WinWaitActive(L"[CLASS:GSWnd]",L"", 10000)){
		allegro_message("Nao foi possivel localizar a janela do GS");
		return FALSE;
	}
	
	//envia bolinha, até ficar em apresentação, por causa do memory card
	int tela=0;
//	char texto[20];
	inicio_timer=GetTickCount();
	while(true){
		tela=retorna_tela_atual();
		if((tela==848) || ((tela > 0) && (tela < 10))){
			ENVIA1_B;
			break;
		}else{
			//sprintf_s(texto, 20, "TELA:%d", tela);
			renderer.update("AGUARDE...");
			ENVIA1_B;
		}

		fim_timer=GetTickCount();
		if((fim_timer - inicio_timer) > (TEMPOAGUARDA * 2)){
			renderer.update("ERRO ESPERANDO TELA"); Sleep(5000);
			return false;
			break;
		}
		Sleep(1000);
	}

	return TRUE;
}


//====================================================================================
//reinicia o jogo para entrar na apresentação
//====================================================================================
BOOL reinicia_jogo(BOOL devolve){
	if (devolve){
		pede_desculpa();
	}

	//move o ponteiro do mouse
	int width=0, height=0;
	if (get_desktop_resolution(&width, &height) == 0){     	
		AU3_MouseMove(width+100,height+100, 0);
	}

	//fecha caso esteja aberto
	if(AU3_ProcessExists(L"pcsx2.exe")){						
		Sleep(2000);
		AU3_ProcessClose(L"pcsx2.exe");
		Sleep(2000);
	}

	//aguarda o programa fechar, e abre novamente
	if(AU3_ProcessWaitClose(L"pcsx2.exe", 5000)){
		ShellExecute(NULL, "open", "Launcher.exe", "", ".\\", SW_SHOWNORMAL);
		Sleep(5000);
	}

	//chama o loader e aguarda pela janela
	ShellExecute(NULL, "open", "pcsx2.exe", "", ".\\", SW_SHOWNORMAL);
	Sleep(3000);
	if(!AU3_ProcessExists(L"pcsx2.exe")){
		allegro_message("Nao foi possivel abrir o emulador");
		return FALSE;
	}
	
	//seleciona o menu Run CDVD
	long sel_menu=AU3_WinMenuSelectItem(L"[CLASS:PCSX2 Main]",L"",L"&File",L"&Run CD/DVD",L"",L"",L"",L"",L"",L"");
	if(!sel_menu){
		return FALSE;
	}

	//espera pela janela do GS
	if(!AU3_WinWaitActive(L"[CLASS:GSWnd]",L"", 10000)){
		allegro_message("Nao foi possivel localizar a janela do GS");
		return FALSE;
	}

	return TRUE;
}


//====================================================================================
//avisa sobre o erro ocorrido e devolve as fichas
//====================================================================================
_inline void pede_desculpa(void){
	BOOL fala=FALSE;
	pega_ficha(TRUE);
	fala=TRUE;
	if(fala){
		PlaySound(".\\soms\\reinicia.wav", 0, SND_FILENAME | SND_ASYNC);
	}	
}


//===========================================================================================
//abre o processo que tera a memoria lida
//===========================================================================================
HANDLE abre_processo_memoria(void){
	HWND hwnd = FindWindowA("PCSX2 Main", NULL);
	HANDLE hProcess = NULL;
	if (hwnd){
		DWORD proc_id; 
		GetWindowThreadProcessId(hwnd, &proc_id); 
		hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ, FALSE, proc_id); 
	}
	return hProcess;
}

//===========================================================================================
//abre o processo que tera a memoria gravada
//===========================================================================================
HANDLE abre_processo_memoria_write(void){
	HWND hwnd = FindWindowA("PCSX2 Main", NULL);
	HANDLE hProcess = NULL;
	if (hwnd){
		DWORD proc_id; 
		GetWindowThreadProcessId(hwnd, &proc_id); 
		hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, proc_id); 
	}
	return hProcess;
}

//===========================================================================================
//fecha o processo que lê a memoria
//===========================================================================================
void fecha_processo_memoria(HANDLE hProcess){	
	if (hProcess != NULL){
		CloseHandle(hProcess);
	}
}

//==============================================================================
// ajusta o volume em jogo ou demo
//==============================================================================
void ajusta_volume_som(int amplitude){
    int i=0;
    
	renderer.update("AJUSTANDO O SOM");
    //deixa o som em 0 para facilitar o calculo
    for(i=0;i<24;i++){
        AU3_Send(L"{VOLUME_DOWN}", 0);            
    }            
    
    //agora ajusta para o valor especificado    
    for(i=0;i<amplitude;i++){
        AU3_Send(L"{VOLUME_UP}", 0);        
    }
	renderer.update("SOM AJUSTADO");
}

//==============================================================================
// função para desabilitar os joystick para o jogo
//==============================================================================
void desabilita_joy(UCHAR num){
	if((num == 1) || (num == 3)){
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Left Analog X",L"7");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Left Analog Y",L"7");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Right Analog X",L"7");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Right Analog Y",L"7");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"A", L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"B", L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"X", L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Y", L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Left Shoulder",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Left Trigger",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Right Shoulder",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Right Trigger",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Start",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Back",L"0");
	}

	if((num == 2) || (num == 3)){
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Left Analog X",L"7");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Left Analog Y",L"7");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Right Analog X",L"7");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Right Analog Y",L"7");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"A", L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"B", L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"X", L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Y", L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Left Shoulder",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Left Trigger",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Right Shoulder",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Right Trigger",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Start",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Back",L"0");
	}
	Sleep(250);
	ENVIA1_F9; Sleep(250);
	renderer.update("JOY DESABILITADO");
}

//==============================================================================

//==============================================================================
// função para Habilitar os joystick para o jogo
//==============================================================================
void habilita_joy(UCHAR num){
	if((num == 1) || (num == 3)){
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Left Analog X",L"1");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Left Analog Y",L"-2");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Right Analog X",L"3");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Right Analog Y",L"-6");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"A", L"3");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"B", L"2");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"X", L"4");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Y", L"1");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Left Shoulder",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Left Trigger",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Right Shoulder",L"8");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Right Trigger",L"6");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Start",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD1", L"Back",L"0");
	}
	if((num == 2) || (num == 3)){
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Left Analog X",L"1");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Left Analog Y",L"-2");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Right Analog X",L"3");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Right Analog Y",L"-6");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"A", L"3");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"B", L"2");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"X", L"4");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Y", L"1");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Left Shoulder",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Left Trigger",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Right Shoulder",L"8");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Right Trigger",L"6");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Start",L"0");
		AU3_IniWrite( L"xbox360cemu.ini", L"PAD2", L"Back",L"0");
	}
	Sleep(250);
	ENVIA1_F9; Sleep(250);
	renderer.update("JOY HABILITADO");
}

//==============================================================================
//lê o titulo da janela do GS e retorno o numero do slot atual
//==============================================================================
_inline int retorna_slot_atual(void){
	HWND teste_hwnd=0;
	int total_caracter=0, retorno=0;
	char texto_retornado[MAX_PATH];
	
	teste_hwnd=GetForegroundWindow();
	if(teste_hwnd != NULL){
		total_caracter=GetWindowTextA(teste_hwnd, texto_retornado, MAX_PATH);
		if(total_caracter){
			texto_retornado[0]=texto_retornado[total_caracter-1];
			texto_retornado[1]='\0';
			retorno=atoi(texto_retornado);
		}
	}

	return retorno;
}


//=================================================================================
//lê o titulo da janela do GS e retorno o numero que aparece entra /xxxxx/ barras
//=================================================================================
_inline int retorna_tela_atual(void){
	HWND teste_hwnd=0;
	int total_caracter=0, retorno=0, c=0;
	char texto_retornado[100], numero[30];
	
	numero[0]='\0';
	try{
		teste_hwnd=GetForegroundWindow();
		if(teste_hwnd != NULL){
			total_caracter=GetWindowTextA(teste_hwnd, texto_retornado, 100);
			if(total_caracter > 2){
				for(int a=0; a < total_caracter; a++){
					if(texto_retornado[a] == '/'){
						c=0;
						for(int b=(a+1); b < total_caracter; b++){
							if(texto_retornado[b] != '/'){
								numero[c]=texto_retornado[b];
								c+=1;
							}else{
								break;
							}
						}
						numero[c]='\0';
						break;
					}
				}
			}
		}
		if(strlen(numero) > 0){
			retorno=atoi(numero);
		}
	}catch(std::string s){
		MessageBoxA(NULL, s.c_str(), "Erro", 0);
	}
	return retorno;
}


//==============================================================================
//corrige o nivel de dificuldade e o MAX POWER e volume de musica e efeitos/vozes
//==============================================================================
void corrige_dificuldade(void){
	const DWORD PTR_NV0=0x200E1C17;	//dificuldade
	const DWORD PTR_MP1=0x200E1C4B;	//POWER p1 
	const DWORD PTR_MP2=0x200E1C59;	//POWER p2
	const DWORD PTR_BG1=0x200E1CFF;	//volume das musicas
	const DWORD PTR_BG2=0x200E1D18;	//volume das vozes
	
	DWORD END_NV0=0;
	DWORD END_MP1=0;
	DWORD END_MP2=0;
	DWORD END_BG1=0;
	DWORD END_BG2=0;

	HANDLE processo=NULL;

	//abre o processo
	processo=abre_processo_memoria_write();	
	if(!processo){
		renderer.update("ERRO AJUSTA NIVEL"); Sleep(5000);
		return;
	}
	
	//pega os endereços corretos
	ReadProcessMemory(processo, (LPVOID)PTR_NV0, &END_NV0, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)PTR_MP1, &END_MP1, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)PTR_MP2, &END_MP2, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)PTR_BG1, &END_BG1, sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)PTR_BG2, &END_BG2, sizeof(DWORD), 0);

	//grava os novos valores
	WriteProcessMemory(processo, (LPVOID)END_NV0, &NIVEL_DIFICULDADE, sizeof(UCHAR), 0);	//NIVEL
	WriteProcessMemory(processo, (LPVOID)END_MP1, &MAX_POWER, sizeof(UCHAR), 0);	//POWER P1
	WriteProcessMemory(processo, (LPVOID)END_MP2, &MAX_POWER, sizeof(UCHAR), 0);	//POWER P2
	WriteProcessMemory(processo, (LPVOID)END_BG1, &MAX_MUSICAS, sizeof(UCHAR), 0);	//MUSICAS
	WriteProcessMemory(processo, (LPVOID)END_BG2, &MAX_VOZES, sizeof(UCHAR), 0);	//VOZES

	fecha_processo_memoria(processo);
}


//====================================================================================
//lê as menssagens do dvdinfo
//====================================================================================
BOOL espera_dvdinfo(DWORD num_tela_min, DWORD num_tela_max, HANDLE processo1, DWORD tempo){
	const DWORD PT_MSVCR90=0x78520000;
	const DWORD OFFSET_DVD1=0x00097408;
	const DWORD OFFSET_DVD2=0x00097410;
	DWORD OFFSET_CORRETO=0;

	HANDLE processo=NULL;
	BOOL retorno=false;
	char buffer_dvd[50];
	char numero[10];
	UCHAR encontrados=0;
	DWORD inicio_timer=0, fim_timer=0;

	//abre o processo
	if(processo1 == NULL){
		processo=abre_processo_memoria();	
		if(!processo){
			renderer.update("ERRO LE DVD"); Sleep(5000);
			return retorno;
		}
	}else{
		processo=processo1;
	}
	
	//testa qual dos endereços está com a menssagem
	ReadProcessMemory(processo, (LPVOID)(PT_MSVCR90 + OFFSET_DVD1), &OFFSET_CORRETO , sizeof(DWORD), 0);
	ReadProcessMemory(processo, (LPVOID)OFFSET_CORRETO, buffer_dvd , 35, 0);
	if(buffer_dvd[0] != 'D'){
		ReadProcessMemory(processo, (LPVOID)(PT_MSVCR90 + OFFSET_DVD2), &OFFSET_CORRETO , sizeof(DWORD), 0);
		ReadProcessMemory(processo, (LPVOID)OFFSET_CORRETO, buffer_dvd , 35, 0);
		//testa novamente
		if(buffer_dvd[0] != 'D'){
			renderer.update("ERRO ENDERECO DVD"); Sleep(10000);
			return retorno;
		}
	}
	
	inicio_timer=GetTickCount();
	while(true){
		buffer_dvd[0]='\0';
		ReadProcessMemory(processo, (LPVOID)OFFSET_CORRETO, buffer_dvd , 35, 0);
		buffer_dvd[32]='\0';
		for(UCHAR a=24; a<32; a++){
			if(isdigit(buffer_dvd[a])){
				numero[a-24]=buffer_dvd[a];
				encontrados++;
			}
		}

		//caso esteja correto
		if(encontrados){
			numero[9]='\0';
			DWORD dvd_info=(DWORD)atol(numero);
			if((dvd_info >= num_tela_min) && (dvd_info <= num_tela_max)){
				retorno= true;
				break;
			}
			encontrados=0;
		}
		
		//sai se não encontrar no tempo especificado
		fim_timer=GetTickCount();
		if((fim_timer-inicio_timer) > tempo){
			retorno=false;
			break;
		}
		Sleep(1000);
	}
	
	//só fecha se tiver sido aberto aqui
	if(processo1==NULL){fecha_processo_memoria(processo);}
	return retorno;
}

//====================================================================================
// função que libera os vetores globais
//====================================================================================
void limpa_memoria(void){   
    //finaliza o timer
    KillTimer (meu_hWnd, ID_GAMEKILLER_1);
}


//====================================================================================
//retorna os enderecos base de pcsx2.exe e msvcrt90.dll
//====================================================================================
bool retorna_enderecos_base(void){
	DWORD END_BASE_MSVCRT=0;							//ponteiro para a base
	DWORD END_BASE_EXE=0;								//ponteiro para a base

	const DWORD PT_BASE_EXECUTAVEL_WIN7=0x71D2C584;
	const DWORD PT_BASE_MSVCRT90_WIN7=0x7773AD30;
	
	const DWORD OFFSET_MSVCRT=0x00097408;

	const DWORD PT_BASE_EXECUTAVEL_WIN5=0x00400000;
	const DWORD PT_BASE_MSVCRT90_WIN5=0x00252560;

	HANDLE processo=NULL;

	//abre o processo
	processo=abre_processo_memoria();	
	if(!processo){
		renderer.update("ERRO LENDO BASE"); Sleep(5000);
		return false;
	}
	
	//pega o endereço base da dll msvcrt90.dll
	if(renderer.get_versao() > 5){
		ReadProcessMemory(processo, (LPVOID)PT_BASE_MSVCRT90_WIN7, &END_BASE_MSVCRT , sizeof(DWORD), 0);
		if(END_BASE_MSVCRT){
			ReadProcessMemory(processo, (LPVOID)(END_BASE_MSVCRT + OFFSET_MSVCRT), &END_BASE_MSVCRT , sizeof(DWORD), 0);
		}else{
			return false;
		}
		ReadProcessMemory(processo, (LPVOID)PT_BASE_EXECUTAVEL_WIN7, &END_BASE_EXE , sizeof(DWORD), 0);
		if(!END_BASE_EXE){
			return false;
		}			
	}else{
		ReadProcessMemory(processo, (LPVOID)PT_BASE_MSVCRT90_WIN5, &END_BASE_MSVCRT , sizeof(DWORD), 0);
		if(END_BASE_MSVCRT){
			ReadProcessMemory(processo, (LPVOID)(END_BASE_MSVCRT + OFFSET_MSVCRT), &END_BASE_MSVCRT , sizeof(DWORD), 0);			
			END_BASE_EXE=PT_BASE_EXECUTAVEL_WIN5;
		}else{
			return false;
		}
	}

	fecha_processo_memoria(processo);
	return true;
}