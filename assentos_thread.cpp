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

struct Log
{
    int operacao;
    int id_usuario;
    int assento_escolhido;
    std::vector<Assento> mapa_assentos;
};

enum Estado
{
    LIVRE,
    RESERVADO
};

// VARIAVEIS GLOBAIS
std::vector<struct Assento> assentos;
std::list<struct Log> buffer_log;
std::string nome_arquivo;
FILE *arquivo;
HANDLE hMutex;
int qtd_assentos;
bool fim_threads = false;

// Código 1 da documentação do trabalho
void visualizaAssentos(int id_usuario)
{
    if (WaitForSingleObject(hMutex, INFINITE) != WAIT_FAILED)
    {
        buffer_log.push_back({1, id_usuario, -1, assentos});
    }
    ReleaseMutex(hMutex);
}

// Código 2 da documentação do trabalho
int alocaAssentoLivre(Assento *assento, int id_usuario)
{
    std::vector<Assento>::iterator it;

    if (WaitForSingleObject(hMutex, INFINITE) != WAIT_FAILED)
    {
        for (it = assentos.begin(); it != assentos.end(); it++)
        {
            if (it->estado == LIVRE)
            {
                it->estado = RESERVADO;
                it->id_usuario = id_usuario;
                *assento = *it;
                buffer_log.push_back({2, id_usuario, it->posicao, assentos});
                return 1;
            }
        }
        buffer_log.push_back({2, id_usuario, -1, assentos});
    }
    ReleaseMutex(hMutex);
    return 0;
}

// Código 3 da documentação do trabalho
int alocaAssentoDado(Assento assento, int id_usuario)
{
    if (WaitForSingleObject(hMutex, INFINITE) != WAIT_FAILED)
    {
        if (assentos[assento.posicao - 1].estado == LIVRE)
        {
            assentos[assento.posicao - 1].estado = RESERVADO;
            assentos[assento.posicao - 1].id_usuario = id_usuario;
            buffer_log.push_back({3, id_usuario, assento.posicao, assentos});
            return 1;
        }
        buffer_log.push_back({3, id_usuario, assento.posicao, assentos});
    }
    ReleaseMutex(hMutex);
    return 0;
}

// Código 4 da documentação do trabalho
int liberaAssento(Assento assento, int id_usuario)
{
    if (WaitForSingleObject(hMutex, INFINITE) != WAIT_FAILED)
    {
        if (assentos[assento.posicao - 1].id_usuario == id_usuario)
        {
            assentos[assento.posicao - 1].estado = LIVRE;
            assentos[assento.posicao - 1].id_usuario = 0;
            buffer_log.push_back({4, id_usuario, assento.posicao, assentos});
            return 1;
        }
        buffer_log.push_back({4, id_usuario, assento.posicao, assentos});
    }
    ReleaseMutex(hMutex);
    return 0;
}

void thread1(void *arg)
{
    // salva seu identificador unico int tid = * (int*)
    int tid = *(int *)arg;

    srand(time(NULL) + tid);

    Assento assento_local = {LIVRE, 0, rand() % qtd_assentos + 1};

    // visualiza mapa de assentos
    visualizaAssentos(tid);

    // tenta alocar um assento livre alocaAssentoLivre(&assento, tid);
    alocaAssentoLivre(&assento_local, tid);

    // visualiza mapa de assentos
    visualizaAssentos(tid);

    _endthread();
}

void thread2(void *arg)
{
    // salva seu identificador unico int tid = * (int*)
    int tid = *(int *)arg;

    srand(time(NULL) + tid);

    // inicializa com um assento
    Assento assento_local = {LIVRE, 0, rand() % qtd_assentos + 1};

    // visualiza mapa de assentos
    visualizaAssentos(tid);

    // tenta alocar um assento especifico alocaAssentoDado(assento, tid);
    alocaAssentoDado(assento_local, tid);

    // visualiza mapa de assentos
    visualizaAssentos(tid);
    _endthread();
}

void thread3(void *arg)
{
    // salva seu identificador unico int tid = * (int*)
    int tid = *(int *)arg;

    srand(time(NULL) + tid);

    Assento assento_local = {LIVRE, 0, rand() % qtd_assentos + 1};

    // visualiza mapa de assentos
    visualizaAssentos(tid);

    // tenta alocar um assento livre
    alocaAssentoLivre(&assento_local, tid);

    // visualiza mapa de assentos visualizaAssentos();
    visualizaAssentos(tid);

    // libera alocacao liberaAssento(assento, tid);
    assento_local = {RESERVADO, 0, rand() % qtd_assentos + 1};
    liberaAssento(assento_local, tid);
    // visualiza mapa de assentos visualizaAssentos();
    _endthread();
}

std::string logString(struct Log l)
{
    std::ostringstream oss;

    oss << l.operacao << "," << l.id_usuario << ",";
    if (l.assento_escolhido != -1)
    {
        oss << l.assento_escolhido;
    }
    oss << "[";
    for (std::vector<Assento>::iterator it = l.mapa_assentos.begin(); it != l.mapa_assentos.end(); it++)
    {
        oss << it->id_usuario;
        if (!(it + 1 == l.mapa_assentos.end()))
        {
            oss << ",";
        }
    }
    oss << "]\n";

    return oss.str();
}

void threadLog(void *arg)
{
    int tid = *(int *)arg;

    struct Log log;

    while (buffer_log.size() > 0 || !fim_threads)
    {
        if (buffer_log.size() == 0)
            continue;
        if (WaitForSingleObject(hMutex, INFINITE) != WAIT_FAILED)
        {
            log = buffer_log.front();
            buffer_log.pop_front();
        }
        ReleaseMutex(hMutex);

        std::cout << logString(log);

        fprintf(arquivo, "%s", logString(log).c_str());
    }

    _endthread();
}

int main(int argc, char *argv[])
{
    HANDLE consumidor;
    HANDLE usuario[3];

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

    hMutex = CreateMutex(NULL, FALSE, NULL);

    fopen_s(&arquivo, nome_arquivo.data(), "w");

    if (arquivo == NULL)
    {
        std::cout << "Nao foi possivel criar o arquivo!" << std::endl;
        system("pause");
        exit(1);
    }

    for (int i = 0; i < qtd_assentos; i++)
    {
        assentos.push_back({LIVRE, 0, i + 1});
    }

    std::cout << "Nome do arquivo: " << nome_arquivo << std::endl;
    std::cout << "Quantidade de assentos: " << qtd_assentos << std::endl
              << std::endl;

    int tids[4] = {0, 1, 2, 3};
    consumidor = (HANDLE)_beginthread(threadLog, 0, &tids[4]);
    usuario[0] = (HANDLE)_beginthread(thread1, 0, &tids[1]);
    usuario[1] = (HANDLE)_beginthread(thread2, 0, &tids[2]);
    usuario[2] = (HANDLE)_beginthread(thread3, 0, &tids[3]);

    WaitForMultipleObjects(3, usuario, TRUE, INFINITE);

    if (WaitForSingleObject(hMutex, INFINITE) != WAIT_FAILED)
    {
        fim_threads = true;
    }
    ReleaseMutex(hMutex);

    WaitForSingleObject(consumidor, INFINITE);

    fclose(arquivo);

    std::cout << std::endl;
    for (std::vector<Assento>::iterator it = assentos.begin(); it != assentos.end(); it++)
    {
        std::cout << ' ' << it->id_usuario << ' ';
    }
    std::cout << std::endl;

    system("pause");
    return 0;
}