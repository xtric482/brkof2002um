class Player{
private:
	bool jogando;
	unsigned char estagio;
	int vitorias;
	unsigned char cpu1;
	unsigned char cpu2;
	unsigned char cpu3;

public:
	Player(); //construtor
	void zera_tudo(void);
	void set_jogando(bool valor);
	void set_estagio();
	void set_vitorias();
	void set_cpu1(unsigned char num);
	void set_cpu2(unsigned char num);
	void set_cpu3(unsigned char num);
	bool get_jogando();
	unsigned char get_estagio();
	int get_vitorias();
	unsigned char get_cpu1(void);
	unsigned char get_cpu2(void);
	unsigned char get_cpu3(void);
};	//não esquecer o ponto e virgula
