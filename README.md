# 🔐 AOBTool — Extração Automatizada de Chaves Criptográficas em Ransomwares

> **Trabalho de Conclusão de Curso** · Bacharelado em Sistemas de Informação  
> Universidade Federal de Uberlândia — Faculdade de Computação  
> Autor: **Gabriel Alves Gama** · Orientador: Dr. Diego Nunes Molinos · 2025

---

## 📌 Sobre o Projeto

Este repositório contém o código-fonte da ferramenta **AOBTool**, desenvolvida como parte do TCC *"Extração Automatizada de Chaves Criptográficas em Ransomwares: Uma Abordagem Forense Baseada em AOB"*.

A ferramenta realiza **varredura de memória volátil em tempo de execução**, localizando e extraindo chaves criptográficas utilizadas por ransomwares antes que sejam descartadas da memória — sem que a vítima perca o acesso aos seus arquivos.

---

## 🧠 Como Funciona

Ransomwares criptografam arquivos e mantêm a chave temporariamente na memória RAM durante o processo de cifragem. A AOBTool explora essa janela temporal por meio de uma técnica chamada **AOB (Array of Bytes)**: uma assinatura binária extraída via engenharia reversa que identifica exatamente onde a chave está armazenada no processo em execução.

```
Ransomware inicia  →  Chave gerada em RAM  →  AOBTool encontra o padrão
       ↓                                              ↓
  Cifra arquivos        ←←←←←←←←←←       Extrai a chave (em ms)
```

### Fluxo da AOBTool

1. Aguarda o processo-alvo aparecer (`GetProcessIdByName`)
2. Suspende o processo para consistência de memória (`SuspendProcess`)
3. Obtém o endereço base do módulo (`GetModuleBaseAddress`)
4. Varre a memória buscando o padrão AOB (`FindPatternInProcess`)
5. Lê o ponteiro da chave e extrai os 32 bytes
6. Salva a chave em arquivo binário e retoma o processo

---

## 📊 Resultados Experimentais

Foram realizadas **10 execuções independentes**, comparando o tempo de extração da chave com o tempo de cifragem do ransomware:

| Execução | Tempo de criptografia (ms) | Busca do pattern (ms) | Leitura da chave (ms) |
|:--------:|:--------------------------:|:---------------------:|:---------------------:|
| 1        | 0.0821518                  | 0.0447                | 0.0012                |
| 2        | 0.0712812                  | 0.0444                | 0.0012                |
| 3        | 0.0613382                  | 0.0444                | 0.0012                |
| 4        | 0.0681346                  | 0.0448                | 0.0012                |
| 5        | 0.0707624                  | 0.0449                | 0.0012                |
| 6        | 0.0637167                  | 0.0589                | 0.0013                |
| 7        | 0.0514660                  | 0.0434                | 0.0013                |
| 8        | 0.0562920                  | 0.0430                | 0.0012                |
| 9        | 0.0913421                  | 0.0462                | 0.0012                |
| 10       | 0.0833063                  | 0.0429                | 0.0012                |

✅ **Em 100% das execuções**, a chave foi extraída com sucesso antes do ransomware finalizar a cifragem.

---

## 🛠️ Ferramentas Utilizadas

| Ferramenta | Função |
|---|---|
| **IDA Free 9.2** | Análise estática e decompilação do binário |
| **Cheat Engine 7.6** | Inspeção dinâmica de memória e validação do AOB |
| **Scylla v0.9.8** | Dump do processo em memória |
| **IDA-Fusion (plugin)** | Geração automática de assinaturas AOB |
| **Visual Studio 2022** | Compilação da AOBTool (C++) |
| **Oracle VirtualBox 7.2.2** | Ambiente isolado para execução segura |

---

## 💻 Código da AOBTool

A ferramenta foi implementada em **C++ (x86)** usando diretamente as APIs do Windows, sem dependências externas.

### Estrutura principal

```
AOBTool/
├── FindPatternInProcess()     # Varredura por padrão AOB na memória
├── GetModuleBaseAddress()     # Endereço base dinâmico (compatível com ASLR)
├── GetProcessIdByName()       # Localização do processo-alvo por nome
├── SuspendProcess()           # Pausa todas as threads do processo
├── ResumeProcess()            # Retoma a execução após a extração
└── main()                     # Coordena todo o fluxo
```

### Assinatura AOB utilizada

```cpp
const char* pattern = "\xBA\x00\x00\x00\x00\x8B\xC8\xE8\x00\x00\x00\x00\xBA";
const char* mask    = "x????xxx????x";
```

A máscara `?` representa bytes variáveis (endereços que mudam por ASLR), tornando a busca robusta entre diferentes execuções.

---

## ⚙️ Como Compilar e Executar

> ⚠️ **Atenção:** Este projeto é estritamente para fins acadêmicos e forenses. Execute apenas em ambientes controlados e isolados.

### Requisitos

- Windows 10 (x86/x64)
- Visual Studio 2022 com suporte a C++
- Permissões de administrador

### Compilação

1. Abra o projeto no **Visual Studio 2022**
2. Selecione a configuração `Release | x86`
3. Compile com `Ctrl+Shift+B`

### Execução

```
1. Inicie a AOBTool com privilégios de administrador
2. Execute o processo-alvo (ransomware em ambiente isolado)
3. A ferramenta detecta automaticamente o processo e inicia a varredura
4. A chave extraída é salva em: C:\Users\<usuario>\Desktop\Dump\chave_lida.bin
5. Pressione F8 para encerrar
```

---

## 🔬 Metodologia

O trabalho seguiu as etapas:

```
Revisão da literatura
        ↓
Desenvolvimento da amostra sintética de ransomware (C++, x86)
        ↓
Dump de memória com Scylla + análise estática no IDA Free
        ↓
Identificação da rotina de geração de chave (sub_DD3B80)
        ↓
Geração da assinatura AOB via plugin IDA-Fusion
        ↓
Validação dinâmica com Cheat Engine
        ↓
Implementação e testes da AOBTool
        ↓
Análise quantitativa dos tempos de extração vs. cifragem
```

A **amostra sintética** foi desenvolvida com base em comportamentos documentados do **WannaCry** e **LockBit**, preservando características reais como geração dinâmica de chaves, uso de rotinas criptográficas e descarte do material sensível após a cifragem.

---

## 📚 Referências Principais

- MAZUR, A. H. B. et al. *Descriptografando: Experimento em análise de memória volátil aplicada à defesa contra ransomware.* SBSeg 2024.
- LEE, H.-W. *Cryptography module detection and identification mechanism on malicious ransomware software.* Journal of Convergence on Internet of Things, 2023.
- LIGH, M. H. et al. *Malware Analyst's Cookbook.* Wiley, 2011.
- IBM Security. *X-Force Threat Intelligence Index.* 2024.
- NIST. *Guide to Integrating Forensic Techniques into Incident Response.* SP 800-86, 2006.

---

## ⚠️ Aviso Legal

Este projeto foi desenvolvido exclusivamente para fins **acadêmicos e de pesquisa em segurança cibernética**. Todo o desenvolvimento e execução foi realizado em **ambiente virtual isolado, sem conexão com a internet**, utilizando uma **amostra sintética** desenvolvida pelo próprio autor.

O uso desta ferramenta em sistemas sem autorização expressa é ilegal. Os autores não se responsabilizam por qualquer uso indevido.

---

## 📄 Publicação

A monografia completa está disponível neste repositório: [`ExtraçãoAutomatizadaChaves.pdf`](./ExtraçãoAutomatizadaChaves.pdf)

Versão publicada institucionalmente:
- https://repositorio.ufu.br/handle/123456789/48151

---

<div align="center">
  <sub>Universidade Federal de Uberlândia · Faculdade de Computação · Monte Carmelo - MG · 2025</sub>
</div>
