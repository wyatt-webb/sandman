FROM python:3.9.21-alpine3.19
WORKDIR /code
ENV FLASK_APP=sandman_web/__init__.py
ENV FLASK_RUN_HOST=0.0.0.0
RUN apk add --no-cache gcc musl-dev linux-headers docker-cli
COPY ./pip_requirements.txt pip_requirements.txt
RUN pip install -r pip_requirements.txt
EXPOSE 5000
COPY . .
CMD ["flask", "run"]