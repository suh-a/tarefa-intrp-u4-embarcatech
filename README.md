# 📌 Projeto: Contador com Interrupções e LED RGB

## 🎯 **Objetivo e Funcionalidade**
Este projeto tem como objetivo apresentar uma contagem numérica de **0 a 9** em uma matriz de LEDs **5x5**, utilizando **interrupções (IRQ)** para controle via botões. O sistema também inclui um **LED RGB** que altera sua cor e comportamento conforme as interações do usuário.

### 🔹 **Funcionamento:**
1. **Inicialmente**, o LED RGB vermelho pisca **5 vezes por segundo**.
2. **Ao pressionar o botão A (GPIO 5)**:
   - A contagem é **incrementada**.
   - O LED RGB muda para **verde**, piscando **3 vezes por segundo**.
   - Os LEDs vermelho e azul são **desligados**.
3. **Ao pressionar o botão B (GPIO 6)**:
   - A contagem é **decrementada**.
   - O LED RGB muda para **azul**, piscando **3 vezes por segundo**.
   - Os LEDs vermelho e verde são **desligados**.

---
## 📊 **Fluxograma do Funcionamento**
![image](https://github.com/user-attachments/assets/ee364cbf-0d50-4b22-bbcd-af5a5f30d0de)


---
## 🛠️ **Passo a Passo do Desenvolvimento**
### 📌 **1. Criar o repositório no GitHub**
- Criar um novo repositório no GitHub para armazenar os arquivos do projeto.

### 📝 **2. Confecção do Código no Visual Studio Code**
- Desenvolver o código em **C/C++** utilizando as bibliotecas do Raspberry Pi Pico.
- Implementar **interrupções (IRQ)** para os botões e **debounce** para evitar leituras incorretas.
- Configurar a **matriz de LEDs WS2812** para exibir os números corretamente.
- Criar a lógica para controle do **LED RGB**.

### 🔬 **3. Testes na Placa**
- Compilar e **carregar o código** na placa **BitDogLab**.
- Verificar a resposta dos botões e o comportamento do LED RGB.
- Ajustar o código conforme necessário.

### 📄 **4. Confecção do Arquivo README**
- Criar um arquivo **README.md** (este documento) para documentar o projeto.

### 🎥 **5. Gravação do Vídeo**
- Demonstrar o funcionamento do projeto.
- Explicar o código e os conceitos utilizados.

---
## 🎯 **Conclusão**
Este projeto demonstra o uso de **interrupções** para controle eficiente de eventos assíncronos, o que melhora o desempenho e a responsividade do sistema. A combinação da **matriz de LEDs WS2812** e do **LED RGB** adiciona elementos visuais que tornam o projeto interativo e intuitivo. 🚀

📌 **Tecnologias utilizadas:** C/C++, Raspberry Pi Pico SDK, WS2812, Interrupções (IRQ).


