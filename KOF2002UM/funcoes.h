//cores para a menssagens creditos
enum CORES {
    VERMELHO = 0xffff0000,
    VERDE = 0xff00ff00,
    AZUL = 0xff0000ff,
    BRANCO = 0xffffffff,
    PRETO = 0xff000000
};

enum BOTOES{
	BUTTON1=0x0001,
	BUTTON2=0x0002,
	BUTTON3=0x0004,
	BUTTON4=0x0008,
	BUTTON5=0x00000010l,
	BUTTON6=0x00000020l,
	BUTTON7=0x00000040l,
	BUTTON8=0x00000080l,
	BUTTON9=0x00000100l,
	BUTTON10=0x00000200l,
	BUTTON11=0x00000400l,
	BUTTON12=0x00000800l,
	BUTTON13=0x00001000l,
	BUTTON14=0x00002000l,
	BUTTON15=0x00004000l
};


//******************************************************************************
//prototipos de funções
//******************************************************************************
void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);

BOOL configura_joysticks(void);
BOOL carrega_configuracao(void);
UCHAR aguarda_deposito_ficha(void);
UCHAR espera_selecionar_player1(void);	//nova
UCHAR espera_selecionar_player2(void);	//nova
UCHAR espera_selecionar_player3(void);	//nova
UCHAR espera_terminar_jogo_p1(void);	//nova
UCHAR espera_terminar_jogo_p2(void);	//nova
UCHAR espera_terminar_jogo_p3(void);	//nova
UCHAR aguarda_assinar_continue(UCHAR num_player);	//nova
UCHAR carrega_slot_numero(char num_slot);	//nova
UCHAR espera_tela_inicio_luta(void);	//nova
UCHAR espera_ordenar_chars(UCHAR num_player);		//nova
UCHAR seleciona_modo_inicial(UCHAR num_player);		//nova
void corrige_dificuldade(void);	//nova
UCHAR trata_gameover(void);	//nova
BOOL espera_dvdinfo(DWORD num_tela_min, DWORD num_tela_max, HANDLE processo1, DWORD tempo); //nova
void gera_random_char(UCHAR num);	//nova
bool retorna_enderecos_base(void); //nova

//funções para leitura de memoria
HANDLE abre_processo_memoria(void);
HANDLE abre_processo_memoria_write(void);
void fecha_processo_memoria(HANDLE hProcess);

_inline void pega_ficha(BOOL tira_poe);

void limpa_memoria(void);
void GetJoyMin( int joy);	//não usada por enquanto

BOOL inicia_jogo(void);
BOOL reinicia_jogo(BOOL devolve);
_inline void pede_desculpa(void);
void ajusta_volume_som(int amplitude);

void desabilita_joy(UCHAR num);
void habilita_joy(UCHAR num);
_inline int retorna_slot_atual(void);	//nova
_inline int retorna_tela_atual(void);	//nova