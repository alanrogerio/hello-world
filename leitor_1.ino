#include <Pn532NfcReader.h>
#include <PN532_HSU.h>

int8_t mIncidenceMatrix[] = {-1,  0,  0,  0,  0,  0,  0,  0,  0,
                              1, -1, -1,  0,  0,  0,  0,  0,  0, 
                              0,  1,  0, -1,  0,  0,  0,  0,  0,
                              0,  0,  1,  0,  0,  0,  0,  1, -1,
                              0,  0,  0,  1, -1,  0,  0,  0,  0, 
                              0,  0,  0,  0,  1, -1,  0, -1,  0,
                              0,  0,  0,  0,  0,  1, -1,  0,  0,
                              0,  0,  0,  0,  0,  0,  1,  0,  0,
                              0,  0,  0,  0,  0,  0,  0,  0,  1};
      
uint16_t mStartingTokenVector[] = {1,0,0,0,0, 0, 0, 0, 0};

uint32_t tagId1 = 0xFF;
uint32_t tagId2 = 0xFF;
uint32_t tagId3 = 0xFF;

const int buttonAccept = 6;
const int buttonReject = 5;

bool tagReadyToContinue = false;

// --------------------------Iniciando as comunicações HSU -----------------------------------

//Rotines related with the configuration of the RFID reader PN532      
PN532_HSU pn532hsu(Serial1);
NfcAdapter nfc1 = NfcAdapter(pn532hsu);

//Creation of the reader and PNRD objects
Pn532NfcReader* reader1 = new Pn532NfcReader(&nfc1);
Pnrd pnrd1 = Pnrd(reader1,9,9);


//Rotines related with the configuration of the RFID reader PN532      
PN532_HSU pn532hsu2(Serial2);
NfcAdapter nfc2 = NfcAdapter(pn532hsu2);
//Creation of the reader and PNRD objects
Pn532NfcReader* reader2 = new Pn532NfcReader(&nfc2);
Pnrd pnrd2 = Pnrd(reader2,9,9);

//Rotines related with the configuration of the RFID reader PN532      
PN532_HSU pn532hsu3(Serial3);
NfcAdapter nfc3 = NfcAdapter(pn532hsu3);
//Creation of the reader and PNRD objects
Pn532NfcReader* reader3 = new Pn532NfcReader(&nfc3);
Pnrd pnrd3 = Pnrd(reader3,9,9);
// -----------------------------------------------------------------------

void setup() {  
  //Initialization of the communication with the reader and with the computer Serial bus 
  Serial.begin(9600); 
  reader1->initialize();    
  reader2->initialize();     
  reader3->initialize();      

  //Iniciação dos botões
  pinMode(buttonAccept, INPUT);
  pinMode(buttonReject, INPUT);

  pnrd1.setIncidenceMatrix(mIncidenceMatrix);
  pnrd1.setTokenVector(mStartingTokenVector);

  //Setting of the classic PNRD approach  
  pnrd1.setAsTagInformation(PetriNetInformation::TOKEN_VECTOR);
  pnrd1.setAsTagInformation(PetriNetInformation::ADJACENCY_LIST);

  pnrd2.setAsTagInformation(PetriNetInformation::TOKEN_VECTOR);
  pnrd2.setAsTagInformation(PetriNetInformation::ADJACENCY_LIST);

  pnrd3.setAsTagInformation(PetriNetInformation::TOKEN_VECTOR);
  pnrd3.setAsTagInformation(PetriNetInformation::ADJACENCY_LIST);


  
  Serial.print("\nInicio do programa");
}

void loop() {
  delay(500);
    //Máquina 1
  if(pnrd1.saveData() == WriteError::NO_ERROR){
        tagId1 = pnrd1.getTagId();
        Serial.print("\nMáquina 1 - Peça detectada");
        Serial.print("\nA Tag: ");
        Serial.print(tagId1,HEX);
        Serial.println(" foi configurada com sucesso. Siga para a proxima antena");
  };

 // Máquina 2
// Leitura da tag
  ReadError readError2 = pnrd2.getData();
  //Se a leitura foi bem sucedida...
  if(readError2 == ReadError::NO_ERROR){
    FireError fireError2;
      // Verifica se é uma nova etiqueta
      if(tagId2 != pnrd2.getTagId()){
        tagId2 = pnrd2.getTagId();
        Serial.print("\nMáquina 2 - Peça detectada"); 
        Serial.print("\nCódigo identificador: "); 
        Serial.println(tagId2, HEX); 
        tagReadyToContinue = false;
        //Realização do disparo da transição T0
        FireError fireError2 = pnrd2.fire(0);

        switch(fireError2){
            case FireError::NO_ERROR :

              //Atualiza a tag
              if(pnrd2.saveData() == WriteError::NO_ERROR){
                Serial.println("Peça pronta para o teste");
                return;
              }else{
                Serial.println("Erro na atualização da tag.");
                return;
              }
              return;

            case FireError::PRODUCE_EXCEPTION :
                Serial.println("Erro: Excessão gerada. A peça seguiu pelo caminho errado.");
                break;

            case FireError::CONDITIONS_ARE_NOT_APPLIED :
            break;          
        }
      }else{
        //Habilita as transições T1 ou T2 em caso de aprovação ou rejeição da peça
        pnrd2.conditionUpdate(1, digitalRead(buttonAccept) && !tagReadyToContinue);
        pnrd2.conditionUpdate(2, digitalRead(buttonReject) && !tagReadyToContinue);

        //Caso esteja habilitada.. dispara transição T1
        fireError2 = pnrd2.fire(1);

          switch (fireError2){
            case FireError::NO_ERROR :
              Serial.println("Peça aprovada.");

              //Atualizando a tag
              if(pnrd2.saveData() == WriteError::NO_ERROR){
                Serial.println("Pronta para prosseguir para o estoque.");
                tagReadyToContinue = true;
              }else{
                Serial.println("Erro na atualização da tag");
              }
            return;

            case FireError::PRODUCE_EXCEPTION :
              Serial.println("Erro: Excessão gerada. A peça seguiu pelo caminho errado.");
              return;

            case FireError::CONDITIONS_ARE_NOT_APPLIED :
            break;
          }

           //Caso esteja habilitada.. dispara transição T2
        fireError2 = pnrd2.fire(2);

          switch (fireError2){
            case FireError::NO_ERROR :
              Serial.println("Peça rejeitada.");

              //Atualizando a tag
              if(pnrd2.saveData() == WriteError::NO_ERROR){
                Serial.println("Pronta para o retrabalho.");
                tagReadyToContinue = true;
              }else{
                Serial.println("Erro na atualização da tag");
              }
            return;

            case FireError::PRODUCE_EXCEPTION :
              Serial.println("Erro: Excessão gerada. A peça seguiu pelo caminho errado.");
              return;

            case FireError::CONDITIONS_ARE_NOT_APPLIED :
            break;
          }
      }
    
  }

     
            // Máquina 3
            // Leitura da tag
            ReadError readError3 = pnrd3.getData();
                   
            //Se a leitura foi bem sucedida
            if(readError3 == ReadError::NO_ERROR){
              FireError fireError3;
 

              //verifica se é uma nova tag
               if(tagId3 != pnrd3.getTagId()){
                    tagId3 = pnrd3.getTagId();
                    Serial.print("\nMáquina 3 - Peça detectada"); 
                    Serial.print("\nCódigo identificador: "); 
                    Serial.println(tagId3, HEX);

                    //Dispara a transição t3
                    FireError fireError3 = pnrd3.fire(3);

                    switch (fireError3){
                        case FireError::NO_ERROR :
                         //Atualizando a tag
                          if(pnrd3.saveData() == WriteError::NO_ERROR){
                               Serial.println("Pronta para o estoque.");
                               return;
                         }else{
                               Serial.println("Erro na atualização da tag");
                               return;
                          }

                        case FireError::PRODUCE_EXCEPTION :
                           Serial.println("Erro: Excessão gerada. A peça seguiu pelo caminho errado.");
                        return;

                        case FireError::CONDITIONS_ARE_NOT_APPLIED :
                        break;
                    }
                     
                }
            }
delay(500);               
}
