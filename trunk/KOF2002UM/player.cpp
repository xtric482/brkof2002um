#include "player.h"

//construtor
Player::Player():jogando(false), estagio(0), vitorias(0), cpu1(0), cpu2(0), cpu3(0){
}

//zera todos os dados
void Player::zera_tudo(void){
	jogando=false;
	estagio=0;
	vitorias=0;
	cpu1=0;
	cpu2=0;
	cpu3=0;
}

//atribui os valores
void Player::set_jogando(bool valor){
	jogando=valor;
}

void Player::set_estagio(){
	estagio+=1;
}

void Player::set_vitorias(){
	vitorias+=1;
}

void Player::set_cpu1(unsigned char num){
	cpu1=num;
}

void Player::set_cpu2(unsigned char num){
	cpu2=num;
}

void Player::set_cpu3(unsigned char num){
	cpu3=num;
}

//retorna os valores
bool Player::get_jogando(){
	return jogando;
}

unsigned char Player::get_estagio(){
	return estagio;
}

int Player::get_vitorias(){
	return vitorias;
}

unsigned char Player::get_cpu1(void){
	return cpu1;
}

unsigned char Player::get_cpu2(void){
	return cpu2;
}

unsigned char Player::get_cpu3(void){
	return cpu3;
}
