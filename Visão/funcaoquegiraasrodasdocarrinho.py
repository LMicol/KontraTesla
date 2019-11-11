anguloReto = 90   # angulo que ta indo reto
maxVelocidade = 128  #velocidade maxima

def funcaoPraGirarARodaDoCarrinhoBemComentada(angulo):
    diferencaAngulos = abs(anguloReto - angulo)  #diferença entre os angulos, o quanto tem que mover  abs=módulo
    if angulo == anguloReto: diferencaAngulos = angulo #se for o mesmo angulo, conserva

    if angulo < anguloReto: lado = "e" #vê de que lado tem que diminuir
    else:                   lado = "d"

    #regra de tres pra achar a porção que tem que diminuir, diminui da velocidade max, round pra arredondar
    novaVelocidade = round(maxVelocidade - (diferencaAngulos * maxVelocidade) / anguloReto)

    #retorna a nova velocidade de acordo com o lado que vira
    if lado == "e": return novaVelocidade, maxVelocidade
    else:           return maxVelocidade, novaVelocidade

i = 100
e, d = funcaoPraGirarARodaDoCarrinhoBemComentada(i)
print(e, d)