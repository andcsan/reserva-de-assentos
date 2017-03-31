#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <ctime>
#include <Windows.h>
#include <process.h>

#define QTD_ASSENTOS_PADRAO 5

struct Assento
{
    enum Estado estado;
    int id_usuario;
    int posicao;
};

enum Estado
{
    LIVRE,
    RESERVADO
};

// VARIAVEIS GLOBAIS
FILE *arquivo;
int qtd_assentos;
std::vector<struct Assento> assentos;
std::string nome_arquivo;

// Código 1 da documentação do trabalho
void visualizaAssentos(int id_usuario)
{
}

// Código 2 da documentação do trabalho
int alocaAssentoLivre(int id_usuario)
{
    std::vector<Assento>::iterator it;
    for (it = assentos.begin(); it != assentos.end(); it++)
    {
        if (it->estado == LIVRE)
        {
            it->estado = RESERVADO;
            it->id_usuario = id_usuario;
            return 1;
        }
    }
}

// Código 3 da documentação do trabalho
int alocaAssentoDado(Assento assento, int id_usuario)
{
    if (assentos[assento.posicao - 1].estado == LIVRE)
    {
        assentos[assento.posicao - 1].estado = RESERVADO;
        assentos[assento.posicao - 1].id_usuario = id_usuario;
        return 1;
    }
    return 0;
}

// Código 4 da documentação do trabalho
int liberaAssento(Assento assento, int id_usuario)
{
    if (assento.id_usuario == id_usuario)
    {
        assentos[assento.posicao - 1].estado = LIVRE;
        assentos[assento.posicao - 1].id_usuario = 0;
        return 1;
    }
    return 0;
}

void geraAcoes(std::string string_log)
{
    int codigo = stoi(string_log.substr(0, 1));
    int id_usuario = stoi(string_log.substr(2, 1));
    int posicao_assento;

    if (codigo != 1 && codigo != 2)
    {
        posicao_assento = stoi(string_log.substr(4, 1));
    }

    if (codigo == 1)
    {
        visualizaAssentos(id_usuario);
    }
    else if (codigo == 2)
    {
        alocaAssentoLivre(id_usuario);
    }
    else if (codigo == 3)
    {
        alocaAssentoDado(assentos[posicao_assento - 1], id_usuario);
    }
    else
    {
        liberaAssento(assentos[posicao_assento - 1], id_usuario);
    }
}

int main(int argc, char *argv[])
{

    if (argc == 1)
    {
        nome_arquivo = "log.txt";
        qtd_assentos = QTD_ASSENTOS_PADRAO;
    }
    else
    {
        nome_arquivo = argv[1];
        qtd_assentos = atoi(argv[2]);
    }

    for (int i = 0; i < qtd_assentos; i++)
    {
        assentos.push_back({LIVRE, 0, i + 1});
    }

    fopen_s(&arquivo, nome_arquivo.data(), "r");

    if (arquivo == NULL)
    {
        std::cout << "Nao foi possivel abrir o arquivo!" << std::endl;
        system("pause");
        exit(1);
    }

    std::cout << "Nome do arquivo: " << nome_arquivo << std::endl;
    std::cout << "Quantidade de assentos: " << qtd_assentos << std::endl
              << std::endl;
    char string_log[50];
    std::string aux;
    while ((fgets(string_log, sizeof(string_log), arquivo)) != NULL)
    {
        printf("%s", string_log);
        aux = string_log;
        geraAcoes(string_log);
    }
    fclose(arquivo);

    std::cout << std::endl;

    std::size_t pos = aux.find("[");
    aux = aux.substr(pos);
    boolean flag = true;
    int i = 1;

    std::cout << std::endl;
    for (std::vector<Assento>::iterator it = assentos.begin(); it != assentos.end(); it++)
    {
        std::cout << ' ' << it->id_usuario << ' ';
        if ((stoi(aux.substr(i, 1)) != it->id_usuario))
        {
            flag = false;
        }
        i += 2;
    }
    std::cout << std::endl
              << aux << std::endl;

    if (flag)
        std::cout << "SUCESSO" << std::endl;
    else
        std::cout << "FALHA" << std::endl;

    system("pause");
    return 0;
}