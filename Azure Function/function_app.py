import os
import azure.functions as func
import logging
import json
from datetime import datetime
import pandas as pd
from azure.cosmos import CosmosClient
import uuid

# Inicializa a Azure Function App
app = func.FunctionApp(http_auth_level=func.AuthLevel.FUNCTION)

@app.route(route="getLatestData", methods=["GET"])
def get_latest_data(req: func.HttpRequest) -> func.HttpResponse:
    try:
        # Obter as variáveis de ambiente para o Cosmos DB
        cosmos_url = os.getenv('COSMOS_DB_URL')
        cosmos_key = os.getenv('COSMOS_DB_KEY')
        database_name = os.getenv('COSMOS_DB_DATABASE_NAME')
        container_name = os.getenv('COSMOS_DB_CONTAINER_NAME')

        # Criar cliente CosmosDB
        try:
            client = CosmosClient(cosmos_url, credential=cosmos_key)
            database = client.get_database_client(database_name)
            container = database.get_container_client(container_name)
        except Exception as e:
            logging.error(f"Erro ao conectar com o CosmosDB: {str(e)}")
            return func.HttpResponse(
                json.dumps({"status": "Falha na conexão com CosmosDB", "erro": str(e)}),
                mimetype="application/json",
                status_code=500
            )

        # Query para pegar o dado mais recente
        query = "SELECT * FROM c ORDER BY c.dataHoraUTC DESC OFFSET 0 LIMIT 1"
        items = list(container.query_items(query, enable_cross_partition_query=True))

        if not items:
            return func.HttpResponse(
                json.dumps({"message": "Nenhum dado encontrado"}),
                mimetype="application/json",
                status_code=404
            )

        # Retornar o registro mais recente
        latest_data = items[0]
        logging.info(f"Último registro recuperado: {latest_data}")

        return func.HttpResponse(
            json.dumps(latest_data),
            mimetype="application/json",
            status_code=200
        )

    except Exception as e:
        logging.error(f"Erro ao processar a solicitação: {str(e)}")
        return func.HttpResponse(
            json.dumps({"erro": f"Erro ao processar a solicitação: {str(e)}"}),
            mimetype="application/json",
            status_code=400
        )


@app.route(route="saveNodeMCUdata", methods=["POST"])
def saveNodeMCUdata(req: func.HttpRequest) -> func.HttpResponse:
    try:
        # Obter as variáveis de ambiente para o Cosmos DB
        cosmos_url = os.getenv('COSMOS_DB_URL')
        cosmos_key = os.getenv('COSMOS_DB_KEY')
        database_name = os.getenv('COSMOS_DB_DATABASE_NAME')
        container_name = os.getenv('COSMOS_DB_CONTAINER_NAME')

        # Criar cliente CosmosDB
        try:
            client = CosmosClient(cosmos_url, credential=cosmos_key)
            database = client.get_database_client(database_name)
            container = database.get_container_client(container_name)
        except Exception as e:
            logging.error(f"Erro ao conectar com o CosmosDB: {str(e)}")
            return func.HttpResponse(
                json.dumps({"status": "Falha na conexão com CosmosDB", "erro": str(e)}),
                mimetype="application/json",
                status_code=500
            )

        # Obter os dados da requisição (enviados pelo NodeMCU)
        req_body = req.get_json()
        temperatura = req_body.get('temperatura')
        umidade = req_body.get('umidade')

        # Validação dos dados recebidos
        if temperatura is None or umidade is None:
            raise ValueError("Dados de temperatura ou umidade ausentes")

        # Adicionar timestamp
        timestamp = datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')

        # Criar o objeto para inserir no CosmosDB
        leitura = {
            "id": str(uuid.uuid4()),  # Gerar um ID único
            "temperatura": temperatura,
            "umidade": umidade,
            "dataHoraUTC": timestamp  # Adiciona timestamp UTC
        }

        # Inserir os dados no CosmosDB
        container.create_item(leitura)

        # Log da inserção
        logging.info(f"Dados inseridos no CosmosDB: {leitura}")

        # Criar um dataframe usando Pandas (para validação ou tratamento posterior)
        df = pd.DataFrame([leitura])
        logging.info(f"DataFrame criado: {df}")

        # Retornar sucesso
        return func.HttpResponse(
            json.dumps({"status": "Dados recebidos e salvos com sucesso", "leitura": leitura}),
            mimetype="application/json",
            status_code=200
        )

    except Exception as e:
        # Captura erros e loga
        logging.error(f"Erro ao processar a solicitação: {str(e)}")
        return func.HttpResponse(
            json.dumps({"erro": f"Erro ao processar a solicitação: {str(e)}"}),
            mimetype="application/json",
            status_code=400
        )
