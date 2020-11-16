FROM runsafesecurity-docker-alkemist-lfr.jfrog.io/centos:7 AS lfr-files

FROM registry.access.redhat.com/ubi8/ubi

ENV YUMOPT="dnf install -y"

ENV LFR_ROOT_PATH=/lfr
COPY --from=lfr-files /usr/src/lfr ${LFR_ROOT_PATH}

# 
# Setup repos and update package manager.
#
RUN dnf update -y \
    && $YUMOPT dnf-plugins-core \
    && subscription-manager register --username ${RHEL_SUB_USERNAME} --password ${RHEL_SUB_PASSWORD} --auto-attach \
    && subscription-manager repos --enable "codeready-builder-for-rhel-8-x86_64-rpms" \
    && $YUMOPT https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm \
    && dnf update -y && dnf clean all && rm -r /var/cache/dnf && dnf upgrade -y

#
# Configure and install Apache Arrow libs. Currently, only the latest release rpm
# completes the dependency installation process.
#
RUN rpm --import https://apache.bintray.com/arrow/centos/RPM-GPG-KEY-apache-arrow \
        && $YUMOPT https://apache.bintray.com/arrow/centos/8/apache-arrow-release-latest.rpm \
	&& dnf update -y \
	&& $YUMOPT arrow-devel \
	&& $YUMOPT parquet-devel

#
# Build tools, misc deps
#
RUN $YUMOPT python36 \
	&& $YUMOPT python3-numpy \
	&& $YUMOPT yaml-cpp-devel \
	&& $YUMOPT cmake \
	&& $YUMOPT gcc-toolset-9 \
	&& $YUMOPT gcc-c++

RUN $YUMOPT gtest-devel \
    && $YUMOPT gmock-devel 

#
# Import tip with build system, source, scripts, etc.
#
ADD . /usr/local/tip

#
# Build tip and install
#
RUN cd /usr/local/tip/build \
    && cmake .. -DCONTAINER=ON \
    && TRAPLINKER_EXTRA_LDFLAGS="--traplinker-static-lfr" ${LFR_ROOT_PATH}/scripts/lfr-helper.sh make -j8 \
    && TRAPLINKER_EXTRA_LDFLAGS="--traplinker-static-lfr" ${LFR_ROOT_PATH}/scripts/lfr-helper.sh make install \
    && cd .. \
    && rm -rf /usr/local/tip/build

#
# Setup default entrypoint
#
#ENTRYPOINT ["python3.6", "/usr/local/tip/parse_and_translate.py"]
