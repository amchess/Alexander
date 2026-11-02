import os
import glob
import argparse

def leggi_file_sicuro(filepath):
    """Tenta di leggere un file gestendo diverse codifiche (UTF-8, Windows-1252, Latin-1)"""
    codifiche = ['utf-8', 'cp1252', 'latin-1']
    
    for codifica in codifiche:
        try:
            with open(filepath, 'r', encoding=codifica) as f:
                return f.read()
        except UnicodeDecodeError:
            continue
            
    # Se tutte le codifiche falliscono (raro), legge ignorando gli errori
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        return f.read()

def unifica_file(output_filename, recursive):
    # Trova tutti i file .h e .cpp
    pattern_h = "**/*.h" if recursive else "*.h"
    pattern_cpp = "**/*.cpp" if recursive else "*.cpp"
    
    files = glob.glob(pattern_h, recursive=recursive) + glob.glob(pattern_cpp, recursive=recursive)
    
    # Rimuove il file di output dalla lista dei file da unificare (per evitare loop)
    output_path = os.path.basename(output_filename)
    files = [f for f in files if os.path.basename(f) != output_path]
    
    # Ordina i file per avere un output coerente
    files.sort()

    if not files:
        print("Nessun file .cpp o .h trovato nella directory corrente.")
        return

    print(f"Trovati {len(files)} file. Inizio l'unificazione in '{output_filename}'...")

    with open(output_filename, "w", encoding="utf-8") as outfile:
        for filepath in files:
            display_path = filepath.replace("\\", "/") 
            
            outfile.write(f"// ============================================================================\n")
            outfile.write(f"// FILE ORIGINALE: {display_path}\n")
            outfile.write(f"// ============================================================================\n\n")
            
            # Usa la funzione sicura per leggere il contenuto
            contenuto = leggi_file_sicuro(filepath)
            outfile.write(contenuto)
                
            outfile.write("\n\n") # Spaziatura finale tra un file e l'altro

    print("Fatto!")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Unifica file .cpp e .h in un unico file.")
    parser.add_argument("-o", "--output", default="unified.cpp", help="Nome del file di output (default: unified.cpp)")
    parser.add_argument("-r", "--recursive", action="store_true", help="Cerca anche nelle sottocartelle")
    
    args = parser.parse_args()
    
    unifica_file(args.output, args.recursive)