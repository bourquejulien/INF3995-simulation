FROM debian:bullseye-slim as build
LABEL argos-example.version="0.1"

# Install common dependencies
RUN apt update && apt install -y \
    sudo \
    build-essential \
    git \
    dpkg \ 
    pkg-config \
    software-properties-common \
    && rm -rf /var/lib/apt/lists/*

# Install ARGoS dependencies
RUN apt update && apt install -y \
    wget \
    freeglut3-dev \
    qtbase5-dev \
    qtchooser \
    qt5-qmake \
    qtbase5-dev-tools \
    libxi-dev \
    libxmu-dev \
    libfreeimage-dev \
    libfreeimageplus-dev \
    liblua5.2-dev \
    lua5.2 \
    liblua5.3-dev \
    lua5.3 \
    libboost-filesystem-dev \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# Install WebViz dependencies
RUN apt update && apt install cmake git zlib1g-dev libssl-dev -y

# Add dummy argument to force rebuild starting from that point
ARG UPDATE_ARGOS=unknown

# Install Argos from source
WORKDIR /root
RUN git clone --depth 1 --branch 3.0.0-beta59 https://github.com/ilpincy/argos3.git &&\
    cd argos3 &&\
    mkdir build_simulator &&\
    cd build_simulator &&\
    cmake ../src -DCMAKE_BUILD_TYPE=Debug \
     -DARGOS_BUILD_FOR=simulator \
     -DARGOS_THREADSAFE_LOG=ON \
     -DARGOS_DYNAMIC_LOADING=ON &&\
    make -j $(nproc)
RUN touch ./argos3/build_simulator/argos3.1.gz &&\
    touch ./argos3/build_simulator/README.html &&\
    cd ./argos3/build_simulator &&\
    make install
RUN chmod +x ./argos3/build_simulator/argos_post_install.sh &&\
    ./argos3/build_simulator/argos_post_install.sh &&\
    echo "\nsource /root/argos3/build_simulator/setup_env.sh\n" >> /.bashrc

FROM build as webviz

WORKDIR /root

RUN git clone --depth 1 https://github.com/bourquejulien/argos3-webviz webviz &&\
    cd webviz &&\
    mkdir build &&\
    cd build &&\
    cmake -DCMAKE_BUILD_TYPE=Release ../src &&\
    make -j $(nproc) &&\
    make install

#################################
#          YOUR CODE            #
#################################

FROM webviz as final

ENV WEBVIZ_PORT=8000
ARG WEB_SOCKET_PORT=3000
ARG SIMULATION_PORT=9854

EXPOSE $WEB_SOCKET_PORT

WORKDIR /root/examples

COPY . .

RUN sed -i "s/port=3000/port=${WEB_SOCKET_PORT}/g" experiments/main_simulation.argos &&\
    sed -i "s/9854/${SIMULATION_PORT}/g" controllers/main_simulation/main_simulation.cpp &&\
    sed -i "s/:3000/:${WEB_SOCKET_PORT}/g" /root/webviz/client/index.html

# Build your code (here examples)
RUN mkdir build && cd build &&\
    cmake -DCMAKE_BUILD_TYPE=Debug .. &&\
    make -j $(nproc)

ENTRYPOINT [ "/root/examples/startup.sh" ]
CMD [ "main_simulation.argos" ]
