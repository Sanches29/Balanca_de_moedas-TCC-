//Programa V4

//Inclusão da biblioteca LiquidCrystal para comunicação com Display 16x2
#include <LiquidCrystal.h>
//Configuração dos pinos do LCD (rs, enable, d4,d5,d6,d7)
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

//declara o pino para interreupção (pino em que o botão MODO será conectado
const byte interruptPin = 3;
//Define porta A0 como pino DT do HX711
#define DT A0
//Define porta A1 como pino SCK do HX711
#define SCK A1

//variáveis globais
long zero=0;//Valor de peso da célula de carga sem peso algum
float val=0;//Valor obtido a partir do peso conhecido (15g)
long leitura=0;//Recebe o retorno da função coleta();
float scale=0;// Fator multiplicativo para tranformar peso em valores de REAL
unsigned int selecao=0;//variavel para seleção do modo
//modos 0,1,2,3 = 5,10,25 e 50 centavos respectivamente ; modo 4 = 1 real ;  modo 5 =  Qualquer tipo de moeda
float w=0; //valor atual
float inicial=0; //valor inicial
float diferenca=0; // diferença entre valor inicial e valor atual
float y=0; //valor em real

//função de configuração inicial
void setup() {
  Serial.begin(9600);  //Inicia porta Serial com BaudRate de 9600
  attachInterrupt(digitalPinToInterrupt(interruptPin), escala, FALLING);//Configura a interrupção (pino, função a ser executada, modo de ativação da interrupção(descida))
  pinMode(interruptPin, INPUT_PULLUP);//define o pino de interrupção como entrada e habilita seu pull up
  pinMode(SCK, OUTPUT);//define o pino SCK como saida
  lcd.begin(16,2);//inicia o LCD 16x2
  lcd.print("Balanca de"); // Escreve "Balança de" na primeira linha
  lcd.setCursor(0,1);//direciona o cursor para segunda linha, posição 0
  lcd.print("Moedas");// Escrevee "moedas"
  delay(1000);//delay de 1 segundo para visualização do titulo
  lcd.clear();//limpa o lcd
  calibracao();//executa função calibração
}
//Função de loop(execução ciclica)
void loop() {
  //De acordo com a variavel "seleção" o programa é direcionado a um método de conversão de peso em valores de Real
  //Nos modos de 0 a 4, após a obteção do valor em peso a partir da formula da celula de carga, tal valor é multiplicado
  //  por uma escala que o transforma em valor em valor de Real
  //No modo 5, após a obtenção do valor em peso a partir da formula da celula de carga, o valor é abatido do anterior,
  // obtendo-se assim um diferencial, o qual é comparado entre deltas de valores de peso, e assim, o valor em Real 
  // correspondente é adicionado à variavel Y, e o o valor "incial" (anterior) é atualizado, para se obter assim um novo diferencial.  
  if(selecao==5){

  delay(1500);// delay para debounce do botão "modo"
  
  leitura=coleta();// executa a função coleta e armazena seu retorno na variável leitura
  w=(((leitura-zero)/val)-2*((leitura-zero)/val));//formula da celula de carga, obtendo correspondencia em peso

  diferenca = w - inicial;//Obtem-se o valor diferencial que será comparado nos deltas de peso:

   if (4.00f <= diferenca && 4.50f >= diferenca){
      y+=0.05;      
    }
    else if(4.69f <= diferenca && 5.20f >= diferenca){
      y+=0.10;      
    }
    else if(7.55f<= diferenca && 7.85f >= diferenca){
      y+=0.25;      
    }
    else if(7.95f <= diferenca && 8.40f >= diferenca){
      y+=0.50;      
    }
    else if(6.86f <= diferenca && 7.36f >= diferenca){
      y+=1.0;
    }

     inicial += diferenca;//valor inicial atualizado
     //Escrita no LCD do valor medido e o modo de seleção
     lcd.setCursor(0,0);
     lcd.print("Valor medido:");
     lcd.setCursor(0,1);
     lcd.print(y,2);
     lcd.print("R$");
     lcd.setCursor(7,1);
     lcd.print("   modo:");
     lcd.print(selecao);
    
  }

  //Valores de seleção diferentes de 5
  else{
  leitura=coleta();// executa a função coleta e armazena seu retorno na variável leitura
  w=(((leitura-zero)/val)-2*((leitura-zero)/val));//formula da celula de carga, obtendo correspondencia em peso
  w= w*scale;//multiplicação do valor em peso pela escala adequada(de acordo com a variavel seleção), transformando
                 //o valor obtido em valor de Real
  //Escrita no LCD do valor medido e o modo de seleção             
  lcd.setCursor(0,0);
  lcd.print("Valor medido:");
  lcd.setCursor(0,1);
  lcd.print(w,2);
  lcd.print("R$");
  lcd.setCursor(7,1);
  lcd.print("   modo:");
  lcd.print(selecao);
}
}
//função calibração, a qual é executada antes de qualquer medição feita na celula de carga
void calibracao(){
  lcd.clear();//limpa o display
  lcd.print("Calibrando");//escreve "Caibrando na primeira linha
  lcd.setCursor(0,1);//direciona o cursor para a segunda linha, posição 0
  lcd.print("Aguarde.."); // escreve "Aguarde" na segunda linha

  //Nesta função "for" faz-se a obtenção de 100 valores lidos na celula de carga sem carga alguma em cima desta
  for(int i=0;i<100;i++){
    leitura=coleta();
    zero+=leitura;//soma de tais valores obtidos na variavel "zero"
  }
  
  zero/=100;//Média dos valores obtidos no "for" e atualização da variavel "zero"
  
  //Neste processo colocaremos um peso conhecido na balança(no caso 15g) a fim de calibrarmos a escala de correspondecia
  //  de medição desta
  lcd.clear();
  lcd.print("Coloque 15g");
  lcd.setCursor(0,1);
  lcd.print("e aguarde");
  leitura = 0; //retira todo valor de leitura que possa estar armazenado na variavel "leitura"
  //A fim de atingir um valor de offset pré-determinado houve a utilização de um while
  while(leitura<1000){
    leitura=coleta();
    leitura=zero-leitura; 
    //A disposição dos valores na operação acima passa a fazer sentido após ter conhecimento de como a função "coleta" funciona
  }

//Após atingir o valor pré-determinado de offset e sair do While a seguinte mensagem é escrita no LCD
  lcd.clear();
  lcd.print("Aguarde...");
  delay(2000);

//Serão mensurados 100 valores do nosso peso conhecido na celula de carga, e estes serão somados
//  na variavel "val"
  for(int i=0;i<100;i++){
    leitura=coleta();
    val+=zero-leitura;
  }

  val=val/100.0;//Média dos valores obtidos em "val"
  val=val/15.0; // Relação da média com o valor correspondente do peso conhecido
  lcd.clear();
}


//A função a seguir é responsável pela coleta de dados da celula de carga a partir do HX711 e retorna um variavel
// do tipo unsigned long
unsigned long coleta(){

   //declaração de variaveis locais que serão utilizadas nesta função
   unsigned long dado;
   unsigned char i;

   pinMode(DT, OUTPUT);//declara DT como saida
   //Coloca-se DT em estado ALTO e SCK em estado BAIXO, ja que ambos precisam estar em estados inversos para a devida obtenção de valores
   //  (Configuração esta recomendada no datasheet do HX711) 
   digitalWrite(DT, HIGH);
   digitalWrite(SCK, LOW);

   dado=0; //retira todo valor de leitura que possa estar armazenado na variavel "dado"

   pinMode(DT, INPUT); //DT agora pode ser declarado como entrada para receber dados do HX711
    
   //Enquanto houver sinal digital disponivel em DT, será feita a leitura de tal entrada 25 vezes. Aqui é configurado o ganho do HX711 para 128,
   // ja que enviaremos 25 pulsos de clock ao pino SCK, e em cada um desses pulsos um bit da entrada é obtido. Após a obtenção de um dado, este bit 
   // é deslocado para a esquerda, e o próximo bit é obtido, até que se complete 24 bits.
   while(digitalRead(DT));

   for(i=0;i<24;i++){
    digitalWrite(SCK, HIGH);
    
    dado=dado<<1;//Deslocamento do bit para a esquerda a partir de bit  shift
    
    digitalWrite(SCK, LOW);
    
    if(digitalRead(DT)){ //caso a leitura na entrada DT seja TRUE(1), há a incrementação da varivel "dado",
                         // e caso for FALSE(0), a variavel "dado" não é incrementada, nos retornando 0, o qual será
      dado++;            // deslocado para a esquerda no loop do "for" como também os valores 1 obtidos.
                         // Assim é feita a diferenciação de valores 1 ou 0 na entrada, obtendo assim
    }                    // a leitura da entrada. 
    
   }
   //Aqui utiliza-se o método de complemento de dois, invertendo o estado apenas do bit mais significativo.
   // Caso o novo valor deste bit seja 1, o sinal negativo é atribuido. Caso contrário, o sinal é positivo.
    digitalWrite(SCK, HIGH);
    dado=dado^0x800000;//XOR com o valor 0x800000 invertendo apenas o MSB
    digitalWrite(SCK, LOW);

    //É importante destacar alguns pontos. Ao pressionar a celula de carga para baixo, valores positivos são obtidos.
    // a partir do complemente do dois tais valores são invertidos, e assim constantemente os valores obtidos serão
    //  NEGATIVOS. No caso a variavel "zero" será sempre negativa, como tambem os dados de leitura, mas, a partir 
    //  da disposição na formula, os valores de leitura serão novamente convertidos a positivos, e os valores de
    //  zero abatidos. EXEMPLO: val+=zero-leitura;

    return(dado); //retorna o valor da variável "dado"
}

//Função que será executada ao ocorrer a interrupção no pino 3.
void escala(){
  delay(300);//Delay para diminuir o Debounce do botão
  
  selecao++; //Incremtento da variável seleção
  if (selecao>5) {//Evitar que a variavel seleção trabalhe apenas nos valores de 0 a 5
    selecao = 0;
  }
  //No Switch a partir da variavel seleção, é escolhido o fator multiplicativo de acordo com o tipo de moeda que
  // foi selecionado para medição. Tais fatores multiplicativos foram obtidos a partir de: ValorEmReal/PesoLidoNaCelula.
  switch(selecao){
    case 0:
     //0.05 centavos
     scale = 0.012;
     break;
    case 1:
    //0.10 centavos
    scale = 0.020;
     break;
    case 2:
    //0.25 centavos
    scale = 0.033;
     break;
    case 3:
    //0.50 centavos
    scale = 0.064;
     break;
    case 4:
    //1.00 reais
    scale = 0.14;
     break;
    case 5:
    //valores de moeda variados
    scale = 1;
    w=0; //valor atual
    inicial=0; //valor inicial
    diferenca=0; // diferença entre estes
    y=0;//valor em REAL
     break; 
  }
  
}
