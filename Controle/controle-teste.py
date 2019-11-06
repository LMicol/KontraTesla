"""

Testa os valores de saída para os motores, dado o posicionamento do volante/joystick

Antes de passar os valores obtidos para a serial, esses serão mapeados para o intervalo [-128,128]
 
"""

def map(value, in_min, in_max, out_min, out_max):
    return round(((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min))


try:
    while(True):
        print("Digite o x (-128, 128): ")
        x = int(input())
        print("Digite o y (-128, 128): ")
        y = int(input())
        if (x >= 0):
            left = y
            right = y - map(x, 0, 128, 0, y)
        else:
            left = y + map(x, 0, 128, 0, y)
            right = y
        print("L=", left)
        print("R=", right, "\n")
except:
    exit