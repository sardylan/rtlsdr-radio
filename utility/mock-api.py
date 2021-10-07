import asyncio
import json

from fastapi import FastAPI, Request, Response
from hypercorn.asyncio import serve
from hypercorn.config import Config

app: FastAPI = FastAPI()


@app.get("/api/public/v1/test")
async def test_get(request: Request):
    response_body: dict = {
        "test": "test"
    }

    response: Response = Response()
    response.status_code = 200
    response.body = json.dumps(response_body).encode()

    return response


@app.post("/api/public/v1/test")
async def test_post(request: Request):
    request_body: dict = await request.json()

    response_body: dict = dict(request_body)

    response: Response = Response()
    response.status_code = 200
    response.body = json.dumps(response_body).encode()

    return response


if __name__ == "__main__":
    asyncio.run(serve(app, Config()))
