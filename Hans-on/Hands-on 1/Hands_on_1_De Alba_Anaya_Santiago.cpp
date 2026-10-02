/*
Universidad De Guadalajara
Ingenieria en computación
Compiladores
Sección: D05
Hands-on 1: Análisis Léxico y Autómatas
Santiago De Alba Anaya
*/ 

// Librerias 
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
using namespace std;

struct Token {
    string tipo;    // MOV - ADD - STO - END - Registro(AL,BL) - Coma(,) - Número
    string lexema;  // El texto como esta
};

// AFD =================================================================================================
// Estados de aceptación: Q_MOV_NUM, Q_ADD_BL, Q_STO_NUM, Q_END
enum Estado {
    Q_INICIO,   // q0

    // MOV R, d -----------------------------------
    Q_M, Q_MO, Q_MOV,   // Letras de MOV
    Q_MOV_SP,   // Espacios del MOV
    Q_MOV_R,    // A o B del registro
    Q_MOV_REG,  // Registro completo
    Q_MOV_SP2,  // Espacios de la coma
    Q_MOV_COMA, // Coma
    Q_MOV_SP3,  // Espacios después de la coma
    Q_MOV_NUM,  // Dirección

    // ADD AL, BL ------------------------------------
    Q_A, Q_AD, Q_ADD,   // Letras de ADD
    Q_ADD_SP,   // Espacios del ADD
    Q_ADD_A, Q_ADD_AL,  // AL 
    Q_ADD_SP2,  // Espacios de la coma
    Q_ADD_COMA, // Coma 
    Q_ADD_SP3,  // Espacios después de la coma
    Q_ADD_B, Q_ADD_BL,  // BL 

    // STO d ---------------------------------------------
    Q_S, Q_ST, Q_STO, // Letras de STO
    Q_STO_SP,   // Espacios de STO
    Q_STO_NUM,  // Dirección

    // END -----------------------------------------------
    Q_E, Q_EN, Q_END,   // END

    // Estado de error -------------------------------------
    Q_ERR
};

bool esEspacio(char c) { return c == ' ' || c == '\t'; }
bool esDigito(char c)  { return c >= '0' && c <= '9'; }

const string MSG_MNEMONICO = "Mnemónico inválido (MOV, ADD, STO, END)";
const string MSG_SOBRAN = "Sobran componentes o espacios al final";

struct ResultadoAFD {
    bool valido;
    string error;
    vector<Token> tokens;
};

ResultadoAFD ejecutarAFD(const string& texto) {
    ResultadoAFD r;
    r.valido = false;
    if (texto.empty()) {
        r.error = "entrada vacía";
        return r;
    }
    Estado estado = Q_INICIO;   // Siempre se empieza en q0
    char letraReg = ' ';        // Primera letra del registro de MOV (A o B)
    string numero;              // Dígitos de la dirección
    for (size_t i = 0; i < texto.size(); i++) { // Recorremos el texto carácter por carácter
        char c = texto[i];
        Estado sig = Q_ERR;                       // Por defecto error
        string msg = "carácter no esperado";      // Se cambia según el estado


        switch (estado) {

        case Q_INICIO:   // La primera letra decide la rama
            if (c == 'M') sig = Q_M;
            else if (c == 'A') sig = Q_A;
            else if (c == 'S') sig = Q_S;
            else if (c == 'E') sig = Q_E;
            else msg = esEspacio(c) ? "No se permiten espacios al inicio" : MSG_MNEMONICO;
            break;

        // Rama MOV --------------------------------------------------------------------
        case Q_M:  if (c == 'O') sig = Q_MO;  else msg = MSG_MNEMONICO; break;
        case Q_MO: if (c == 'V') sig = Q_MOV; else msg = MSG_MNEMONICO; break;

        case Q_MOV: // Al menos un espacio
            if (esEspacio(c)) sig = Q_MOV_SP;
            else msg = "Falta un espacio entre el mnemónico y sus operandos";
            break;
        case Q_MOV_SP:  // Más espacioso el registro (A o B)
            if (esEspacio(c)) sig = Q_MOV_SP;
            else if (c == 'A' || c == 'B') sig = Q_MOV_R;
            else msg = "Registro inválido (AL o BL)";
            break;
        case Q_MOV_R:   // Después de A o B tiene que venir L
            if (c == 'L') sig = Q_MOV_REG;
            else msg = "Registro inválido (AL o BL)";
            break;
        case Q_MOV_REG: // La coma
            if (c == ',') sig = Q_MOV_COMA;
            else if (esEspacio(c)) sig = Q_MOV_SP2;
            else msg = "Falta la coma entre el registro y la dirección";
            break;
        case Q_MOV_SP2:
            if (esEspacio(c)) sig = Q_MOV_SP2;
            else if (c == ',') sig = Q_MOV_COMA;
            else msg = "Falta la coma entre el registro y la dirección";
            break;
        case Q_MOV_COMA: // Espacios opcionales o directamente un dígito
            if (esEspacio(c)) sig = Q_MOV_SP3;
            else if (esDigito(c)) sig = Q_MOV_NUM;
            else msg = "Dirección inválida (dígitos)";
            break;
        case Q_MOV_SP3:
            if (esEspacio(c)) sig = Q_MOV_SP3;
            else if (esDigito(c)) sig = Q_MOV_NUM;
            else msg = "Dirección inválida (dígitos)";
            break;
        case Q_MOV_NUM:  // Solo más dígitos
            if (esDigito(c)) sig = Q_MOV_NUM; else msg = MSG_SOBRAN;
            break;

        // Rama ADD ------------------------------------------------------------------------------
        case Q_A:  if (c == 'D') sig = Q_AD;  else msg = MSG_MNEMONICO; break;
        case Q_AD: if (c == 'D') sig = Q_ADD; else msg = MSG_MNEMONICO; break;

        case Q_ADD:
            if (esEspacio(c)) sig = Q_ADD_SP;
            else msg = "Falta un espacio entre el mnemónico y sus operandos";
            break;
        case Q_ADD_SP:  // ADD solo admite AL como primer operando
            if (esEspacio(c)) sig = Q_ADD_SP;
            else if (c == 'A') sig = Q_ADD_A;
            else msg = "Primer operando inválido (AL)";
            break;
        case Q_ADD_A:
            if (c == 'L') sig = Q_ADD_AL; else msg = "Registro inválido (AL)";
            break;
        case Q_ADD_AL:
            if (c == ',') sig = Q_ADD_COMA;
            else if (esEspacio(c)) sig = Q_ADD_SP2;
            else msg = "Falta la coma entre AL y BL";
            break;
        case Q_ADD_SP2:
            if (esEspacio(c)) sig = Q_ADD_SP2;
            else if (c == ',') sig = Q_ADD_COMA;
            else msg = "Falta la coma entre AL y BL";
            break;
        case Q_ADD_COMA: // ADD solo admite BL como segundo operando
            if (esEspacio(c)) sig = Q_ADD_SP3;
            else if (c == 'B') sig = Q_ADD_B;
            else msg = "segundo operando inválido (se esperaba BL)";
            break;
        case Q_ADD_SP3:
            if (esEspacio(c)) sig = Q_ADD_SP3;
            else if (c == 'B') sig = Q_ADD_B;
            else msg = "segundo operando inválido (se esperaba BL)";
            break;
        case Q_ADD_B:
            if (c == 'L') sig = Q_ADD_BL; else msg = "registro inválido (se esperaba BL)";
            break;
        case Q_ADD_BL:  // Ya no puede seguir nada 
            msg = MSG_SOBRAN;
            break;

        // STO ----------------------------------------------------------------------
        case Q_S:  if (c == 'T') sig = Q_ST;  else msg = MSG_MNEMONICO; break;
        case Q_ST: if (c == 'O') sig = Q_STO; else msg = MSG_MNEMONICO; break;

        case Q_STO:
            if (esEspacio(c)) sig = Q_STO_SP;
            else msg = "Falta un espacio entre el mnemónico y la dirección";
            break;
        case Q_STO_SP:
            if (esEspacio(c)) sig = Q_STO_SP;
            else if (esDigito(c))  sig = Q_STO_NUM;
            else msg = "Dirección inválida (se esperan dígitos)";
            break;
        case Q_STO_NUM:
            if (esDigito(c)) sig = Q_STO_NUM; else msg = MSG_SOBRAN;
            break;

        // END ------------------------------------------------------------------------
        case Q_E:  if (c == 'N') sig = Q_EN;  else msg = MSG_MNEMONICO; break;
        case Q_EN: if (c == 'D') sig = Q_END; else msg = MSG_MNEMONICO; break;
        case Q_END: // No admite nada más
            msg = "END no admite operandos ni espacios adicionales";
            break;

        case Q_ERR: 
            break;
        }

        // Si la transición fue a Q_ERR se rechaza la entrada y se detiene
        if (sig == Q_ERR) {
            // r.error = msg + " --(carácter '" + string(1, c) + "' en la posición " + to_string(i + 1) + ")--"; // Muestra el carácter incorrecto en la posición del comando
            r.error = msg;
            return r;
        }

        // Guardar los tokens de los operandos
        switch (sig) {
            case Q_MOV:r.tokens.push_back(Token{"MOV", "MOV"}); break;
            case Q_ADD:r.tokens.push_back(Token{"ADD", "ADD"}); break;
            case Q_STO:r.tokens.push_back(Token{"STO", "STO"}); break;
            case Q_END:r.tokens.push_back(Token{"END", "END"}); break;

            case Q_MOV_R:letraReg = c; break;   // Si fue A o B
            case Q_MOV_REG:r.tokens.push_back(Token{"REGISTRO", string(1, letraReg) + "L"}); break;
            case Q_ADD_AL:r.tokens.push_back(Token{"REGISTRO", "AL"}); break;
            case Q_ADD_BL:r.tokens.push_back(Token{"REGISTRO", "BL"}); break;

            case Q_MOV_COMA:
            case Q_ADD_COMA:r.tokens.push_back(Token{"COMA", ","}); break;

            case Q_MOV_NUM:
            case Q_STO_NUM: numero += c; break;    // Juntar los dígitos
            default: break;
        }

        estado = sig; // Transición
    }

    bool acepta = (estado == Q_END) || (estado == Q_MOV_NUM) || (estado == Q_ADD_BL) || (estado == Q_STO_NUM);

    if (!acepta) {
        if (r.tokens.empty()) r.error = "instrucción incompleta: mnemónico sin terminar";
        else r.error = "instrucción incompleta: faltan operandos de " + r.tokens[0].lexema;
        return r;
    }

    if (!numero.empty()) r.tokens.push_back(Token{"NUMERO", numero});
    r.valido = true;
    return r;
}

// EL OBJETO INSTRUCCIÓN ===================================================================================

struct Instruccion {
    string operacion; // "MOV", "ADD", "STO" o "END"
    vector<string> registros; // {"AL"}, {"AL","BL"} o {}
    long long direccion; // -1 significa "no tiene" (se imprime null)
};

// Arma el objeto a partir de los tokens y devuelve false si la dirección tiene demasiados dígitos
bool construirObjeto(const vector<Token>& tokens, Instruccion& ins) {
    ins.operacion = tokens[0].lexema;   // el primer token siempre es el mnemónico
    ins.direccion = -1;

    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].tipo == "REGISTRO") {
            ins.registros.push_back(tokens[i].lexema);
        } else if (tokens[i].tipo == "NUMERO") {
            if (tokens[i].lexema.size() > 18) return false;  
            ins.direccion = stoll(tokens[i].lexema);          
        }
    }
    return true;
}

// Imprime el objeto
void imprimirObjeto(const Instruccion& ins) {
    cout << " {" << endl;
    cout << "   operacion: \"" << ins.operacion << "\"," << endl;
    cout << "   registros: [";
    for (size_t i = 0; i < ins.registros.size(); i++) {
        if (i > 0) cout << ", ";
        cout << "\"" << ins.registros[i] << "\"";
    }
    cout << "]," << endl;
    cout << "   direccion: ";
    if (ins.direccion == -1) cout << "null";
    else cout << ins.direccion;
    cout << endl << " }" << endl;
}

// LA MÁQUINA DE MOORE =============================================================================
enum EstadoMoore {
    AL_PREPARAR_DIR, AL_LEER_MEMORIA, AL_CARGAR,    // ruta MOV AL, d
    BL_PREPARAR_DIR, BL_LEER_MEMORIA, BL_CARGAR,    // ruta MOV BL, d
    SUMAR,  // ruta ADD AL, BL
    STO_PREPARAR_DIR, STO_LEER_ACC, STO_ESCRIBIR,   // ruta STO d
    DETENER,    // ruta END
    FIN
};

class MaquinaMoore {
public:
    // Al crear la máquina se elige la ruta según el objeto
    MaquinaMoore(const Instruccion& i) : ins(i), estado(FIN) {
        if (ins.operacion == "MOV") estado = (ins.registros[0] == "AL") ? AL_PREPARAR_DIR : BL_PREPARAR_DIR;
        else if (ins.operacion == "ADD") estado = SUMAR;
        else if (ins.operacion == "STO") estado = STO_PREPARAR_DIR;
        else estado = DETENER;
    }

    bool terminada() const { return estado == FIN; }

    // Emite la salida del estado actual y avanza al siguiente
    string siguientePaso() {
        string salida;
        switch (estado) {
            // MOV AL, d --------------------------------------
            case AL_PREPARAR_DIR:
                salida = "MAR ← " + to_string(ins.direccion);   // el objeto aporta la dirección
                estado = AL_LEER_MEMORIA;
                break;
            case AL_LEER_MEMORIA:
                salida = "MBR ← M[MAR]";
                estado = AL_CARGAR;
                break;
            case AL_CARGAR:
                salida = "AL ← MBR";
                estado = FIN;
                break;

            // MOV BL, d ----------------------------------------
            case BL_PREPARAR_DIR:
                salida = "MAR ← " + to_string(ins.direccion);
                estado = BL_LEER_MEMORIA;
                break;
            case BL_LEER_MEMORIA:
                salida = "MBR ← M[MAR]";
                estado = BL_CARGAR;
                break;
            case BL_CARGAR:
                salida = "BL ← MBR";
                estado = FIN;
                break;

            // ADD AL, BL ------------------------------------------
            case SUMAR:
                salida = "ACC ← AL + BL";
                estado = FIN;
                break;

            // STO d -----------------------------------------------
            case STO_PREPARAR_DIR:
                salida = "MAR ← " + to_string(ins.direccion);
                estado = STO_LEER_ACC;
                break;
            case STO_LEER_ACC:
                salida = "MBR ← ACC";
                estado = STO_ESCRIBIR;
                break;
            case STO_ESCRIBIR:
                salida = "M[MAR] ← MBR";
                estado = FIN;
                break;

            // END ----------------------------------------------------
            case DETENER:
                salida = "HALT ← 1";
                estado = FIN;
                break;

            case FIN:
                break;  // No emite nada
        }
        return salida;
    }

private:
    Instruccion ins;
    EstadoMoore estado;
};

// PROGRAMA PRINCIPAL ==================================================================
// Ejecuta las etapas para una instrucción e imprime 
void procesar(const string& entrada) {
    cout << "\n-----------------------------------------------------------------" << endl;
    cout << "ENTRADA" << "\n - " << entrada << endl;

    // AFD
    cout << "\nVALIDACIÓN MEDIANTE AFD" << endl ;
    ResultadoAFD r = ejecutarAFD(entrada);

    if (!r.valido) {
        cout << " - Instrucción inválida: " << r.error << endl;
        cout << "\n - No se construye el objeto instrucción." << endl;
        cout << " - No se generan microoperaciones." << endl;
        return;  
    }
    cout << " - Instrucción válida." << endl;

    cout << "\nTOKENS RECONOCIDOS" << endl;
    for (size_t i = 0; i < r.tokens.size(); i++)
        cout << " - " << r.tokens[i].tipo << "(\"" << r.tokens[i].lexema << "\")" << endl;

    // Bbjeto instrucción
    Instruccion ins;
    if (!construirObjeto(r.tokens, ins)) {
        cout << "\nError: la dirección tiene demasiados dígitos." << endl;
        cout << "No se generan microoperaciones." << endl;
        return;
    }

    cout << "\nCOMPONENTES IDENTIFICADOS" << endl;
    cout << " - Mnemónico: " << ins.operacion << endl;
    if (ins.operacion == "MOV") {
        cout << " - Registro destino: " << ins.registros[0] << endl;
        cout << " - Dirección de memoria: " << ins.direccion << endl;
        cout << " - Direccionamiento: directo" << endl;
    } else if (ins.operacion == "ADD") {
        cout << " - Registros: " << ins.registros[0] << ", " << ins.registros[1] << endl;
        cout << " - Resultado en: ACC" << endl;
    } else if (ins.operacion == "STO") {
        cout << " - Dirección de memoria: " << ins.direccion << endl;
        cout << " - Direccionamiento: directo" << endl;
    } else {
        cout << " - Sin operandos" << endl;
    }

    cout << "\nOBJETO INSTRUCCIÓN" << endl;
    imprimirObjeto(ins);

    // Máquina de Moore
    cout << "\nMICROOPERACIONES GENERADAS POR MOORE"  << endl;
    MaquinaMoore moore(ins);
    int n = 1;
    while (!moore.terminada()) {
        cout << " " << n << ". " << moore.siguientePaso() << endl;
        n++;
    }
    cout << "\nGeneración terminada." << endl;
    cout << "-----------------------------------------------------------------"  << endl << endl;
}

int main() {
    string linea;
    while (true) {
        cout << "\n======================== ANÁLISIS LÉXICO DE AUTOMATAS ========================";
        cout << "\n\nInstrucción de entrada (Enter para salir): ";
        if (!getline(cin, linea)) break;
        if (!linea.empty() && linea[linea.size() - 1] == '\r') linea.erase(linea.size() - 1);
        if (linea.empty()) break;
        procesar(linea);

        // cout << "\nPresiona Enter para continuar...";
        // cin.get();
        // system("cls");
    }
    return 0;
}