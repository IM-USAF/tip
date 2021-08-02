FROM registry.il2.dso.mil/skicamp/project-opal/opal-operations:vendor-whl AS wheel
FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/ironbank/miniconda:4.9.2 AS builder

USER root

COPY --from=wheel /whl /whl

RUN mkdir /tip
COPY tip_scripts /tip/tip_scripts/
COPY README.md /tip/README.md

WORKDIR /tip

# ARG GITLAB_TOKEN
# RUN git clone https://__token__@code.il2.dso.mil/skicamp/project-opal/opal-operations.git

ENV CONDA_PATH="/opt/conda"
ENV CONDA_CHANNEL_DIR="/local-channels"
ENV ARTIFACT_CHANNEL_DIR=".ci_artifacts/build-metadata/build-artifacts"

ADD $ARTIFACT_CHANNEL_DIR/local_channel.tar $CONDA_CHANNEL_DIR
RUN mv $CONDA_CHANNEL_DIR/local-channel $CONDA_CHANNEL_DIR/tip-package-channel

ENV PATH="${CONDA_PATH}/bin:${PATH}"
RUN conda init && source /root/.bashrc && conda activate base && \
    pip3 install --no-cache-dir /whl/conda_vendor-0.1-py3-none-any.whl && \
    echo "CONDA_CHANNEL_DIR = ${CONDA_CHANNEL_DIR}" && \
    ls ${CONDA_CHANNEL_DIR} && \
    mkdir "${CONDA_CHANNEL_DIR}/singleuser-channel" && \
    python -m conda_vendor local-channels -f /tip/tip_scripts/singleuser/singleuser.yml --channel-root "${CONDA_CHANNEL_DIR}/singleuser-channel" && \
    mkdir "${CONDA_CHANNEL_DIR}/tip-dependencies-channel" && \
    python -m conda_vendor local-channels -f /tip/tip_scripts/conda-mirror/tip_dependency_env.yml --channel-root "${CONDA_CHANNEL_DIR}/tip-dependencies-channel" && \
    ls ${CONDA_CHANNEL_DIR} 

USER 1000

FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0

SHELL ["/usr/bin/bash", "-c"] 

ENV CONDA_ADD_PIP_AS_PYTHON_DEPENDENCY=False
ENV CONDA_PATH="/opt/conda"

ARG NB_USER="jovyan"
ARG NB_UID="1000"
ARG NB_GID="100"

ENV NB_USER="${NB_USER}" \
    NB_UID=${NB_UID} \
    NB_GID=${NB_GID}

RUN groupadd -r ${NB_USER} && useradd -l -r -g ${NB_GID} -u ${NB_UID} ${NB_USER} && mkdir /home/${NB_USER} && \
    chown -R ${NB_USER}:${NB_USER} /home/${NB_USER}

COPY --from=builder --chown=${NB_USER}:${NB_USER} $CONDA_PATH $CONDA_PATH
COPY --from=builder --chown=${NB_USER}:${NB_USER} /local-channels /home/${NB_USER}/local-channels
COPY --chown=${NB_USER}:${NB_USER} conf /home/${NB_USER}/conf
COPY --chown=${NB_USER}:${NB_USER} conf/default_conf/*.yaml /home/${NB_USER}/conf/
COPY --chown=${NB_USER}:${NB_USER} tip_scripts/singleuser/jupyter_notebook_config.py /home/${NB_USER}/.jupyter/
COPY --chown=${NB_USER}:${NB_USER} tip_scripts/single_env/start_jupyter_nb.sh /home/${NB_USER}/user_scripts/

RUN chmod +x /home/${NB_USER}/user_scripts/start_jupyter_nb.sh && \
    rm -rf /usr/share/doc/perl-IO-Socket-SSL/certs/*.enc && \
    rm -rf /usr/share/doc/perl-IO-Socket-SSL/certs/*.pem && \
    rm -r /usr/share/doc/perl-Net-SSLeay/examples/*.pem && \
    rm  /usr/lib/python3.6/site-packages/pip/_vendor/requests/cacert.pem && \
    rm  /usr/share/gnupg/sks-keyservers.netCA.pem && \
    rm -rf ${CONDA_PATH}/conda-meta && \
    rm -rf ${CONDA_PATH}/include

USER ${NB_USER}
RUN mkdir "/home/${NB_USER}/work"

ENV PATH="${CONDA_PATH}/bin:$PATH"
RUN conda init bash && source /home/${NB_USER}/.bashrc
WORKDIR /home/${NB_USER}

RUN conda create -n tip \
    tip \
    jupyter \
    python>3.9 \
    jupyterlab \
    jupyterhub \
    pandas \
    matplotlib \
    pyarrow \
    yaml-cpp \
    pyyaml \
    libpcap \
    arrow-cpp \
    spdlog \
    libtins \
    pip \
    -c /home/${NB_USER}/local-channels/singleuser-channel/local_conda-forge \
    -c /home/${NB_USER}/local-channels/tip-package-channel \
    -c /home/${NB_USER}/local-channels/tip-dependencies-channel/local_conda-forge \
    --offline \
    && rm -rf /home/${NB_USER}/local-channels/singleuser-channel/local_conda-forge \
    && rm -rf /home/${NB_USER}/local-channels/tip-package-channel \
    && rm -rf /home/${NB_USER}/local-channels/tip-dependencies-channel/local_conda-forge

EXPOSE 8888

ENTRYPOINT ["/usr/bin/bash","/home/${NB_USER}/user_scripts/start_jupyter_nb.sh"]

