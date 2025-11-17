#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXN 150
#define NOME_MAX 90
#define VALOR_HE 80

// ========================================================
// ESTRUTURAS
// ========================================================
typedef struct {
    int codigo;
    char nome[NOME_MAX];
} Colaborador;

typedef struct {
    int codigoCol;
    int entrada;
    int fim;
    int extra;
    int foiAprovado; // 1 sim / 0 nao
    char dataRegistro[32];
} Historico;

// Banco em memória
Colaborador base[MAXN];
int usados = 0;

// ========================================================
// FUNÇÕES AUXILIARES
// ========================================================
void pausa() {
    printf("\n(Pressione ENTER...)");
    getchar();
    getchar();
}

void limparEntrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Converte data atual para string
void pegarData(char *dest) {
    time_t agora = time(NULL);
    struct tm *dt = localtime(&agora);
    sprintf(dest, "%02d/%02d/%04d %02d:%02d:%02d",
            dt->tm_mday, dt->tm_mon+1, dt->tm_year + 1900,
            dt->tm_hour, dt->tm_min, dt->tm_sec);
}

// ========================================================
// FUNÇÕES DE ARQUIVO
// ========================================================
void carregarColaboradores() {
    FILE *arq = fopen("func.db", "r");
    if (!arq) return;

    usados = 0;
    while (fscanf(arq, "%d|%[^\n]", &base[usados].codigo, base[usados].nome) == 2) {
        usados++;
        if (usados >= MAXN) break;
    }

    fclose(arq);
}

void salvarColaboradores() {
    FILE *arq = fopen("func.db", "w");
    if (!arq) return;

    for (int i = 0; i < usados; i++) {
        fprintf(arq, "%d|%s\n", base[i].codigo, base[i].nome);
    }

    fclose(arq);
}

// ========================================================
// CONSULTAS
// ========================================================
int existeColaborador(int cod) {
    for (int i = 0; i < usados; i++) {
        if (base[i].codigo == cod) return 1;
    }
    return 0;
}

char* nomeDoColaborador(int cod) {
    for (int i = 0; i < usados; i++) {
        if (base[i].codigo == cod) return base[i].nome;
    }
    return "(desconhecido)";
}

// ========================================================
// CÁLCULO DE HORAS EXTRAS
// ========================================================
int calcularHE(int entrada, int saida) {
    int jornada = saida - entrada;
    if (jornada <= 8) return 0;

    int e = jornada - 8;
    return (e > 2 ? 2 : e);
}

// ========================================================
// CADASTRO
// ========================================================
void novoColaborador() {
    system("clear");
    printf("\n=== CADASTRAR NOVO COLABORADOR ===\n");

    if (usados >= MAXN) {
        printf("Capacidade esgotada.\n");
        pausa();
        return;
    }

    int codigo;
    printf("Código desejado: ");
    scanf("%d", &codigo);
    limparEntrada();

    if (existeColaborador(codigo)) {
        printf("Já existe alguém com este código.\n");
        pausa();
        return;
    }

    printf("Nome completo: ");
    fgets(base[usados].nome, NOME_MAX, stdin);
    base[usados].nome[strcspn(base[usados].nome, "\n")] = 0;

    base[usados].codigo = codigo;
    usados++;

    salvarColaboradores();

    printf("Cadastro concluído!\n");
    pausa();
}

void listarColaboradores() {
    system("clear");
    printf("\n=== LISTA DE COLABORADORES ===\n\n");

    if (usados == 0) {
        printf("Nenhum colaborador registrado.\n");
    } else {
        for (int i = 0; i < usados; i++) {
            printf("%d - %s\n", base[i].codigo, base[i].nome);
        }
    }

    pausa();
}

// ========================================================
// SOLICITAÇÃO DE HE
// ========================================================
void registrarHE() {
    system("clear");
    printf("\n=== REGISTRO DE HORA EXTRA ===\n\n");

    if (usados == 0) {
        printf("Cadastre colaboradores primeiro.\n");
        pausa();
        return;
    }

    listarColaboradores();
    int codigo;
    printf("\nCódigo do colaborador: ");
    scanf("%d", &codigo);

    if (!existeColaborador(codigo)) {
        printf("Código inválido.\n");
        pausa();
        return;
    }

    int escolha;
    printf("\nGestor: Autorizar hora extra?\n1 - Sim\n2 - Não\n-> ");
    scanf("%d", &escolha);

    Historico h;
    h.codigoCol = codigo;
    pegarData(h.dataRegistro);

    if (escolha == 1) {
        h.foiAprovado = 1;

        printf("Hora de entrada (ex: 8): ");
        scanf("%d", &h.entrada);

        printf("Hora de saída (ex: 17): ");
        scanf("%d", &h.fim);

        h.extra = calcularHE(h.entrada, h.fim);

        printf("\nHE calculada: %dh\n", h.extra);
    } else {
        h.foiAprovado = 0;
        h.extra = 0;
        h.entrada = 0;
        h.fim = 0;

        printf("Hora extra negada.\n");
    }

    FILE *arq = fopen("he.db", "a");
    if (arq) {
        fprintf(arq, "%d|%s|%d|%d|%d|%d\n",
                h.codigoCol, h.dataRegistro,
                h.entrada, h.fim, h.extra, h.foiAprovado);
        fclose(arq);
    }

    printf("\nRegistro salvo!\n");
    pausa();
}

// ========================================================
// RELATÓRIOS
// ========================================================
void relatorioIndividual() {
    system("clear");
    printf("\n=== RELATÓRIO INDIVIDUAL ===\n\n");

    listarColaboradores();

    int codigo;
    printf("Informe o código: ");
    scanf("%d", &codigo);

    if (!existeColaborador(codigo)) {
        printf("Código inválido.\n");
        pausa();
        return;
    }

    FILE *arq = fopen("he.db", "r");
    if (!arq) {
        printf("Nenhum registro disponível.\n");
        pausa();
        return;
    }

    char linha[256];
    int totalHE = 0;
    int cont = 0;

    printf("\n--- Registros de %s ---\n\n", nomeDoColaborador(codigo));

    while (fgets(linha, sizeof(linha), arq)) {
        int cod, ent, fim, ex, ap;
        char data[32];

        sscanf(linha, "%d|%[^|]|%d|%d|%d|%d", &cod, data, &ent, &fim, &ex, &ap);

        if (cod == codigo) {
            printf("Data: %s\n", data);
            printf("Entrada: %dh | Saída: %dh\n", ent, fim);
            printf("Extra: %dh | Status: %s\n",
                   ex, ap ? "Aprovado" : "Negado");
            printf("---------------------------------\n");

            totalHE += ex;
            cont++;
        }
    }

    fclose(arq);

    printf("\nTotal de registros: %d\n", cont);
    printf("Total de horas extras: %d\n", totalHE);
    printf("Valor total: R$%d\n", totalHE * VALOR_HE);

    pausa();
}

void relatorioGeral() {
    system("clear");
    printf("\n=== RELATÓRIO GERAL ===\n\n");

    FILE *arq = fopen("he.db", "r");
    if (!arq) {
        printf("Nenhum registro encontrado.\n");
        pausa();
        return;
    }

    char linha[256];

    while (fgets(linha, sizeof(linha), arq)) {
        int cod, ent, fim, ex, ap;
        char data[32];

        sscanf(linha, "%d|%[^|]|%d|%d|%d|%d", &cod, data, &ent, &fim, &ex, &ap);

        printf("[%s] %s | Extra: %dh | %s\n",
               data, nomeDoColaborador(cod), ex,
               ap ? "OK" : "Negado");
    }

    fclose(arq);
    pausa();
}

// ========================================================
// FECHAMENTO
// ========================================================
void fechamentoTotal() {
    system("clear");
    printf("\n=== FECHAR CICLO DE HORAS EXTRAS ===\n");

    int totalHE = 0;

    for (int i = 0; i < usados; i++) {
        FILE *arq = fopen("he.db", "r");
        if (!arq) continue;

        char linha[256];
        int soma = 0;

        while (fgets(linha, sizeof(linha), arq)) {
            int cod, ex;
            char data[32];
            int ent, fim, ap;

            sscanf(linha, "%d|%[^|]|%d|%d|%d|%d",
                   &cod, data, &ent, &fim, &ex, &ap);

            if (cod == base[i].codigo) soma += ex;
        }

        fclose(arq);

        printf("\nColaborador: %s\n", base[i].nome);
        printf("Total HE: %dh\n", soma);
        printf("Pagamento: R$%d\n", soma * VALOR_HE);
        printf("---------------------------\n");

        totalHE += soma;
    }

    printf("\nTOTAL GERAL DE HORAS: %d\n", totalHE);
    printf("VALOR TOTAL: R$%d\n", totalHE * VALOR_HE);

    // apagar registros
    FILE *z = fopen("he.db", "w");
    if (z) fclose(z);

    printf("\nRegistros eliminados.\n");
    pausa();
}

// ========================================================
// MENU PRINCIPAL
// ========================================================
int main() {
    carregarColaboradores();
    int op;

    do {
        system("clear");
        printf("=== SISTEMA DE HORAS EXTRAS v2 ===\n");
        printf("1 - Novo colaborador\n");
        printf("2 - Listar colaboradores\n");
        printf("3 - Registrar hora extra\n");
        printf("4 - Relatório individual\n");
        printf("5 - Relatório geral\n");
        printf("6 - Fechar ciclo\n");
        printf("7 - Sair\n-> ");
        scanf("%d", &op);

        switch(op) {
            case 1: novoColaborador(); break;
            case 2: listarColaboradores(); break;
            case 3: registrarHE(); break;
            case 4: relatorioIndividual(); break;
            case 5: relatorioGeral(); break;
            case 6: fechamentoTotal(); break;
            case 7: break;
            default:
                printf("Opção inválida!\n");
                pausa();
        }

    } while (op != 7);

    return 0;
}
