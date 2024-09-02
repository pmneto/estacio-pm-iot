import azure.functions as func
import requests
import json

def obter_dados_nodemcu():
    # URL do servidor NodeMCU
    url = "http://<endereco-ip-nodemcu>/"

    # Fazer a requisição GET para obter os dados
    response = requests.get(url)

    if response.status_code == 200:
        dados = response.json()
        temperatura = dados['temperatura']
        umidade = dados['umidade']
        
        # Estruturar os dados em um dicionário Python
        resultado = {
            "temperatura": temperatura,
            "umidade": umidade
        }
        
        # Retornar os dados em formato JSON
        return json.dumps(resultado)
    else:
        return json.dumps({"erro": f"Erro ao acessar o NodeMCU: {response.status_code}"})

def main(req: func.HttpRequest) -> func.HttpResponse:
    # Chamando a função que obtém os dados do NodeMCU
    dados_json = obter_dados_nodemcu()

    # Retornando o JSON como resposta da Azure Function
    return func.HttpResponse(dados_json, mimetype="application/json")
