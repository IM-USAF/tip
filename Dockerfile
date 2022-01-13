FROM registry1.dso.mil/ironbank/opensource/metrostar/tip-dependencies:0.0.3 AS tipdependencies
FROM registry1.dso.mil/ironbank/opensource/metrostar/singleuser:torch_1.10.0_v2 AS pytorch
FROM registry1.dso.mil/ironbank/opensource/metrostar/singleuser:0.0.1 AS singleuser

COPY --from=tipdependencies /local_channel /home/jovyan/local_channel
COPY --chown=jovyan:jovyan --from=pytorch /home/jovyan/local-channel /home/jovyan/local-channel


ENV ARTIFACT_DIR=".ci_artifacts/build-metadata/build-artifacts"

WORKDIR /home/jovyan

COPY --chown=jovyan:jovyan $ARTIFACT_DIR/local_channel.tar .
COPY --chown=jovyan:jovyan ./conf /home/jovyan/
COPY --chown=jovyan:jovyan --from=pytorch  /home/jovyan/local-channel/local_channel_env.yaml  /home/jovyan/local-channel/local_channel_env.yaml

RUN source /opt/conda/bin/activate && \
    conda env create -f /home/jovyan/local-channel/local_channel_env.yaml && \
    rm -rf /home/jovyan/local-channel

RUN tar xvf local_channel.tar --strip-components=2 && \
    ls -la && \
    pwd && \
    source /opt/conda/bin/activate && \
    conda activate singleuser && \
    conda install -c file:///home/jovyan/local_channel/ -c file:///home/jovyan/local-channel tip --offline

