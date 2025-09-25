#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ====== Definições ====== */
typedef struct {
    char nome[30];
    char cor[10];   /* dono (cor do exército) */
    int tropas;
} Territorio;

/* ====== Assinaturas ====== */
Territorio* alocarMapa(int n);
void cadastrarTerritorios(Territorio* mapa, int n);
void exibirMapa(const Territorio* mapa, int n);
int  encontrarIndicePorNome(const Territorio* mapa, int n, const char* nomeBuscado);
int  podeAtacar(const Territorio* mapa, int n, int iAtacante, int iDefensor);
void atacar(Territorio* atacante, Territorio* defensor);
void liberarMemoria(Territorio* mapa);

void limparBuffer(void);
void lerLinha(char* destino, int tam);

/* ====== Função principal ====== */
int main(void) {
    srand((unsigned)time(NULL)); /* garante aleatoriedade */

    int n;
    printf("Quantos territorios deseja cadastrar? ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Valor invalido.\n");
        return 1;
    }
    limparBuffer();

    Territorio* mapa = alocarMapa(n);
    if (!mapa) {
        printf("Falha ao alocar memoria.\n");
        return 1;
    }

    cadastrarTerritorios(mapa, n);

    int opcao;
    do {
        printf("\n===== MENU =====\n");
        printf("1) Exibir mapa\n");
        printf("2) Atacar\n");
        printf("0) Sair\n");
        printf("Opcao: ");
        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida.\n");
            limparBuffer();
            continue;
        }
        limparBuffer();

        if (opcao == 1) {
            exibirMapa(mapa, n);
        } else if (opcao == 2) {
            char nomeAtac[30], nomeDef[30];

            printf("\n--- Escolha os territorios para o ataque ---\n");
            exibirMapa(mapa, n);

            printf("\nDigite o NOME do territorio ATACANTE: ");
            lerLinha(nomeAtac, sizeof(nomeAtac));
            int iA = encontrarIndicePorNome(mapa, n, nomeAtac);
            if (iA < 0) { printf("Atacante nao encontrado.\n"); continue; }

            printf("Digite o NOME do territorio DEFENSOR: ");
            lerLinha(nomeDef, sizeof(nomeDef));
            int iD = encontrarIndicePorNome(mapa, n, nomeDef);
            if (iD < 0) { printf("Defensor nao encontrado.\n"); continue; }

            if (!podeAtacar(mapa, n, iA, iD)) {
                continue; /* mensagens de erro já são exibidas em podeAtacar */
            }

            atacar(&mapa[iA], &mapa[iD]);

            printf("\n--- Estado apos o ataque ---\n");
            exibirMapa(mapa, n);
        } else if (opcao == 0) {
            /* sair */
        } else {
            printf("Opcao invalida.\n");
        }
    } while (opcao != 0);

    liberarMemoria(mapa);
    return 0;
}

/* ====== Implementações ====== */

/* Aloca dinamicamente o vetor de territorios */
Territorio* alocarMapa(int n) {
    /* calloc zera memória, útil para evitar lixo inicial */
    Territorio* mapa = (Territorio*)calloc(n, sizeof(Territorio));
    return mapa;
}

/* Cadastro guiado dos territorios */
void cadastrarTerritorios(Territorio* mapa, int n) {
    printf("\n--- Cadastro de Territorios ---\n");
    for (int i = 0; i < n; i++) {
        printf("\nTerritorio %d/%d\n", i+1, n);

        printf("Nome (max 29): ");
        lerLinha(mapa[i].nome, sizeof(mapa[i].nome));

        printf("Cor (dono) (max 9): ");
        lerLinha(mapa[i].cor, sizeof(mapa[i].cor));

        do {
            printf("Tropas (>= 0): ");
            if (scanf("%d", &mapa[i].tropas) != 1) {
                printf("Valor invalido.\n");
                limparBuffer();
                mapa[i].tropas = -1;
            } else if (mapa[i].tropas < 0) {
                printf("Nao pode ser negativo.\n");
            }
        } while (mapa[i].tropas < 0);
        limparBuffer();
    }
}

/* Exibe todos os territorios */
void exibirMapa(const Territorio* mapa, int n) {
    printf("\n%-3s | %-29s | %-9s | %-6s\n", "#", "Nome", "Cor", "Tropas");
    printf("-----------------------------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf("%-3d | %-29s | %-9s | %-6d\n", i, mapa[i].nome, mapa[i].cor, mapa[i].tropas);
    }
}

/* Busca por nome (case sensitive simples) */
int encontrarIndicePorNome(const Territorio* mapa, int n, const char* nomeBuscado) {
    for (int i = 0; i < n; i++) {
        if (strcmp(mapa[i].nome, nomeBuscado) == 0) return i;
    }
    return -1;
}

/* Regras de validação antes do ataque */
int podeAtacar(const Territorio* mapa, int n, int iAtacante, int iDefensor) {
    if (iAtacante < 0 || iAtacante >= n || iDefensor < 0 || iDefensor >= n) {
        printf("Indices invalidos.\n");
        return 0;
    }
    if (iAtacante == iDefensor) {
        printf("Um territorio nao pode atacar a si mesmo.\n");
        return 0;
    }
    if (strcmp(mapa[iAtacante].cor, mapa[iDefensor].cor) == 0) {
        printf("Nao e permitido atacar um territorio da MESMA cor (dono).\n");
        return 0;
    }
    if (mapa[iAtacante].tropas < 2) {
        /* Regra simples: exige pelo menos 2 para atacar (uma fica para manter o territorio) */
        printf("O atacante precisa ter pelo menos 2 tropas para atacar. (Atacante tem %d)\n",
               mapa[iAtacante].tropas);
        return 0;
    }
    return 1;
}

/*
 * Simula a batalha:
 * - Rola um dado (1..6) para cada lado.
 * - Empate favorece o DEFENSOR (estilo WAR).
 * - Se ATACANTE vencer:
 *     - DEFENSOR muda de cor para a do atacante.
 *     - Metade das tropas do ATACANTE (arredondado para baixo, mínimo 1) é movida para o DEFENSOR conquistado.
 *       (Atualiza as tropas do ATACANTE e do DEFENSOR de acordo.)
 * - Se ATACANTE perder:
 *     - ATACANTE perde 1 tropa.
 */
void atacar(Territorio* atacante, Territorio* defensor) {
    int dadoA = 1 + rand() % 6;
    int dadoD = 1 + rand() % 6;

    printf("\nBatalha: %s (%s) ATACA %s (%s)\n",
           atacante->nome, atacante->cor, defensor->nome, defensor->cor);
    printf("Rolagens -> Atacante: %d  |  Defensor: %d\n", dadoA, dadoD);

    if (dadoA > dadoD) {
        printf("Resultado: ATACANTE venceu!\n");

        /* transfere posse e metade das tropas do atacante para o novo territorio */
        int mover = atacante->tropas / 2;
        if (mover < 1) mover = 1;                /* garante ao menos 1 indo para o conquistado */
        if (mover >= atacante->tropas) mover = atacante->tropas - 1; /* deixa ao menos 1 no atacante */

        /* muda dono (cor) do defensor */
        strcpy(defensor->cor, atacante->cor);

        /* tropas do conquistado = tropas movidas do atacante */
        defensor->tropas = mover;

        /* atualiza tropas remanescentes no atacante */
        atacante->tropas -= mover;

        printf("Conquista! %s agora pertence a %s.\n", defensor->nome, defensor->cor);
        printf("Tropas movimentadas: %d (ficam no territorio conquistado)\n", mover);
    } else {
        printf("Resultado: DEFENSOR resistiu. Atacante perde 1 tropa.\n");
        if (atacante->tropas > 0) atacante->tropas -= 1;
        if (atacante->tropas < 0) atacante->tropas = 0;
    }
}

/* Libera a memória alocada dinamicamente */
void liberarMemoria(Territorio* mapa) {
    free(mapa);
}

/* ====== Utilidades de entrada ====== */
void limparBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* descarta até fim de linha */ }
}

void lerLinha(char* destino, int tam) {
    if (fgets(destino, tam, stdin) != NULL) {
        size_t len = strlen(destino);
        if (len > 0 && destino[len - 1] == '\n') destino[len - 1] = '\0';
    } else {
        /* fallback */
        destino[0] = '\0';
    }
}
