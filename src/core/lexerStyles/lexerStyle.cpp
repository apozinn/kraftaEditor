#include "./lexerStyle.hpp"
#include <wx/stc/stc.h>

StyleInfo global_lexer_styles[32] = {
    // === Estilos do Editor (0-15) ===
    {"Default", wxColour(240, 240, 240), "", 10, 0, 0},               // 0: Branco fumê (texto padrão)
    {"Número da Linha", wxColour(150, 150, 150), "", 10, 0, 0},        // 1: Cinza (números de linha)
    {"Chave Correspondente", wxColour(173, 216, 230), "", 10, 0, 0},   // 2: Azul claro (destaque de chaves)
    {"Chave Inválida", wxColour(255, 0, 0), "", 10, 0, 0},            // 3: Vermelho (erro de chave)
    {"Caractere de Controle", wxColour(100, 100, 100), "", 10, 0, 0}, // 4: Cinza escuro (caracteres não imprimíveis)
    {"Guia de Indentação", wxColour(200, 200, 200), "", 10, 0, 0},    // 5: Cinza claro (guias de indentação)
    {"Dica de Chamada", wxColour(220, 220, 220), "", 10, 0, 0},        // 6: Cinza muito claro (tooltips)
    {"Margem de Dobra", wxColour(240, 240, 240), "", 10, 0, 0},        // 7: Branco fumê (fundo da margem)
    {"Dobra Ativa", wxColour(200, 200, 255), "", 10, 0, 0},           // 8: Azul lavanda (destaque de dobra)
    {"Marcador de Dobra", wxColour(180, 180, 180), "", 10, 0, 0},     // 9: Cinza (símbolos +/-)
    {"Espaço em Branco", wxColour(180, 180, 180), "", 10, 0, 0},      // 10: Cinza (espaços visíveis)
    {"Link Ativo", wxColour(255, 100, 100), "", 10, 0, 0},            // 11: Vermelho claro (links clicáveis)
    {"Texto Invisível", wxColour(240, 240, 240), "", 10, 0, 0},       // 12: Branco fumê (texto oculto)
    {"Seleção", wxColour(173, 214, 255), "", 10, 0, 0},               // 13: Azul claro (texto selecionado)
    {"Linha do Cursor", wxColour(220, 240, 255), "", 10, 0, 0},       // 14: Azul muito claro (linha atual)
    {"Margem Direita", wxColour(230, 230, 230), "", 10, 0, 0},        // 15: Cinza (linha de margem)

    // === Estilos de Sintaxe (16-31) ===
    {"Palavra-chave 1", wxColour(86, 156, 214), "", 10, 1, 0},        // 16: Azul (palavras-chave primárias)
    {"Palavra-chave 2", wxColour(136, 120, 195), "", 10, 0, 0},       // 17: Roxo (palavras-chave secundárias)
    {"Comentário", wxColour(106, 153, 85), "", 10, 0, 0},             // 18: Verde (comentários)
    {"Comentário Doc", wxColour(106, 153, 85), "", 10, 2, 0},         // 19: Verde itálico (comentários de documentação)
    {"Comentário de Linha", wxColour(106, 153, 85), "", 10, 0, 0},    // 20: Verde (comentários de linha)
    {"String", wxColour(206, 145, 120), "", 10, 0, 0},                // 21: Marrom claro (strings)
    {"Caractere", wxColour(209, 105, 105), "", 10, 0, 0},             // 22: Vermelho claro (caracteres)
    {"Número", wxColour(181, 206, 168), "", 10, 0, 0},                // 23: Verde pálido (números)
    {"Operador", wxColour(212, 212, 212), "", 10, 1, 0},              // 24: Branco (operadores em negrito)
    {"Identificador", wxColour(220, 220, 220), "", 10, 0, 0},          // 25: Branco fumê (identificadores)
    {"Pré-processador", wxColour(155, 155, 155), "", 10, 0, 0},       // 26: Cinza (diretivas)
    {"Expressão Regular", wxColour(255, 215, 0), "", 10, 0, 0},       // 27: Dourado (regex)
    {"Erro", wxColour(255, 0, 0), "", 10, 0, 0},                     // 28: Vermelho (erros de sintaxe)
    {"Valor", wxColour(211, 84, 0), "", 10, 0, 0},                   // 29: Laranja (valores)
    {"Tipo", wxColour(78, 201, 176), "", 10, 0, 0},                  // 30: Turquesa (tipos)
    {"Variável", wxColour(184, 215, 163), "", 10, 0, 0}              // 31: Verde menta (variáveis)
};