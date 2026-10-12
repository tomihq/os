# =====================================================================
# VM sencilla para jugar con drivers de kernel en C (QEMU + cloud-init)
#
# Requisitos en el host (Debian/Ubuntu):
#   sudo apt install qemu-system-x86 qemu-utils curl openssh-client \
#                    cloud-image-utils      # (o genisoimage / xorriso)
#
# Uso:
#   make start-vm          # primera vez: baja la imagen y prepara todo
#   make load-nulo         # compila src/nulo/ adentro de la VM y lo carga
#   make dmesg-nulo
#   make unload-nulo
#   make ssh               # shell dentro de la VM
#   make stop-vm
#   make help
# =====================================================================

VM_DIR   ?= .vm
IMG_URL  ?= https://cloud.debian.org/images/cloud/bookworm/latest/debian-12-genericcloud-amd64.qcow2
VM_RAM   ?= 1024
VM_CPUS  ?= 2
VM_DISK  ?= 10G
SSH_PORT ?= 2222
VM_USER  := taller

SRC_DIR      ?= src
REMOTE_BUILD := /tmp/taller-drivers
DMESG_LINES  ?= 50

BASE_IMG := $(VM_DIR)/base.qcow2
DISK     := $(VM_DIR)/disk.qcow2
SEED     := $(VM_DIR)/seed.iso
KEY      := $(VM_DIR)/id_ed25519
PIDFILE  := $(VM_DIR)/qemu.pid
LOG      := $(VM_DIR)/console.log

SSH = ssh -i $(KEY) -p $(SSH_PORT) \
      -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
      -o LogLevel=ERROR -o ConnectTimeout=3 $(VM_USER)@127.0.0.1

.PHONY: help start-vm stop-vm wait-vm ssh status clean distclean

help:
	@echo "VM:      start-vm | stop-vm | ssh | status | distclean"
	@echo "Drivers: build-X | load-X | unload-X | reload-X | dmesg-X   (X = carpeta en $(SRC_DIR)/)"
	@echo "Cliente: cliente-X | exec-X | run-X   (compila/corre src/X/main.c; ARGS=\"...\")"
	@echo "Ej:      make load-nulo   /   make run-nulo"

# ---------------------------------------------------------------------
# Preparación de la VM (una sola vez)
# ---------------------------------------------------------------------

$(VM_DIR):
	@mkdir -p $@

$(BASE_IMG): | $(VM_DIR)
	@echo "==> Bajando imagen de Debian 12..."
	@curl -L --fail --progress-bar -o $@.tmp $(IMG_URL) && mv $@.tmp $@

$(KEY): | $(VM_DIR)
	@ssh-keygen -q -t ed25519 -N '' -f $@

# Disco "overlay": la imagen base queda intacta, los cambios van acá.
$(DISK): $(BASE_IMG)
	@qemu-img create -q -f qcow2 -F qcow2 -b $(abspath $(BASE_IMG)) $@ $(VM_DISK)

# cloud-init: crea el usuario, instala toolchain + headers y, si el kernel
# que corre no tiene headers disponibles, reinicia para arrancar el nuevo.
# (El $$ es para que $(uname -r) se evalúe adentro de la VM y no en el host.)
$(SEED): $(KEY)
	@printf '%s\n' \
	  '#cloud-config' \
	  'users:' \
	  '  - name: $(VM_USER)' \
	  '    sudo: ALL=(ALL) NOPASSWD:ALL' \
	  '    shell: /bin/bash' \
	  '    ssh_authorized_keys:' \
	  "      - $$(cat $(KEY).pub)" \
	  'ssh_pwauth: false' \
	  'package_update: true' \
	  'packages: [build-essential, python3, psmisc, linux-image-cloud-amd64, linux-headers-cloud-amd64]' \
	  'runcmd:' \
	  '  - [touch, /var/lib/taller-installed]' \
	  '  - [sh, -c, "test -d /lib/modules/$$(uname -r)/build || reboot"]' \
	  > $(VM_DIR)/user-data
	@printf 'instance-id: taller-vm\nlocal-hostname: taller\n' > $(VM_DIR)/meta-data
	@if command -v cloud-localds >/dev/null; then \
	    cloud-localds $@ $(VM_DIR)/user-data $(VM_DIR)/meta-data; \
	 elif command -v xorriso >/dev/null; then \
	    xorriso -as mkisofs -quiet -o $@ -volid cidata -joliet -rock $(VM_DIR)/user-data $(VM_DIR)/meta-data; \
	 elif command -v genisoimage >/dev/null; then \
	    genisoimage -quiet -o $@ -volid cidata -joliet -rock $(VM_DIR)/user-data $(VM_DIR)/meta-data; \
	 else echo "Falta cloud-localds, xorriso o genisoimage"; exit 1; fi

# ---------------------------------------------------------------------
# Ciclo de vida de la VM
# ---------------------------------------------------------------------

start-vm: $(DISK) $(SEED)
	@if [ -f $(PIDFILE) ] && kill -0 $$(cat $(PIDFILE)) 2>/dev/null; then \
	    echo "La VM ya está corriendo."; \
	 else \
	    echo "==> Arrancando la VM (SSH en localhost:$(SSH_PORT))..."; \
	    qemu-system-x86_64 \
	      -accel kvm -accel tcg -cpu max \
	      -m $(VM_RAM) -smp $(VM_CPUS) \
	      -drive file=$(DISK),if=virtio,format=qcow2 \
	      -drive file=$(SEED),if=virtio,format=raw,media=cdrom,readonly=on \
	      -nic user,hostfwd=tcp:127.0.0.1:$(SSH_PORT)-:22 \
	      -display none -serial file:$(LOG) \
	      -pidfile $(PIDFILE) -daemonize; \
	 fi
	@$(MAKE) --no-print-directory wait-vm

wait-vm:
	@printf 'Esperando a que la VM esté lista (la primera vez tarda unos minutos)'; \
	for i in $$(seq 1 180); do \
	  if $(SSH) "test -f /var/lib/taller-installed && test -d /lib/modules/\$$(uname -r)/build" 2>/dev/null; then \
	    echo ' lista.'; exit 0; \
	  fi; \
	  printf .; sleep 5; \
	done; \
	echo ' timeout. Mirá $(LOG)'; exit 1

stop-vm:
	-@$(SSH) "sudo poweroff" 2>/dev/null || true
	@if [ -f $(PIDFILE) ]; then \
	  pid=$$(cat $(PIDFILE)); \
	  for i in 1 2 3 4 5 6 7 8 9 10; do kill -0 $$pid 2>/dev/null || break; sleep 1; done; \
	  kill $$pid 2>/dev/null || true; rm -f $(PIDFILE); \
	fi
	@echo "VM apagada."

ssh:
	@$(SSH)

status:
	@if [ -f $(PIDFILE) ] && kill -0 $$(cat $(PIDFILE)) 2>/dev/null; then \
	    echo "VM corriendo (pid $$(cat $(PIDFILE)))"; else echo "VM apagada"; fi

# ---------------------------------------------------------------------
# Drivers: src/<nombre>/<nombre>.c  +  src/<nombre>/Makefile (kbuild)
# Se compilan ADENTRO de la VM, contra los headers de su kernel.
# ---------------------------------------------------------------------

build-%: start-vm
	@test -f $(SRC_DIR)/$*/$*.c || { \
	    echo "Falta $(SRC_DIR)/$*/$*.c"; exit 1; }
	@test -f $(SRC_DIR)/$*/Makefile || { \
	    echo "Falta $(SRC_DIR)/$*/Makefile (manifest de kbuild)"; exit 1; }
	@$(SSH) "rm -rf $(REMOTE_BUILD)/$* && mkdir -p $(REMOTE_BUILD)/$*"
	@tar -C $(SRC_DIR)/$* -c . | $(SSH) "tar -C $(REMOTE_BUILD)/$* -x"
	@$(SSH) "make -C $(REMOTE_BUILD)/$*"
	@echo "compilado (adentro de la VM): $(REMOTE_BUILD)/$*/$*.ko"

load-%: build-%
	@$(SSH) "sudo insmod $(REMOTE_BUILD)/$*/$*.ko"
	@echo "cargado: $*.ko -> /dev/$* (si el driver crea ese device)"

# Si el rmmod normal falla (device abierto por un proceso colgado), mata a
# quien lo tenga abierto y fuerza el rmmod.
unload-%:
	@$(SSH) "sudo rmmod $* 2>/dev/null || { sudo fuser -k /dev/$*; sleep 1; sudo rmmod -f $*; }"
	@echo "descargado: $*"

reload-%: build-%
	-@$(SSH) "sudo rmmod $* 2>/dev/null"
	@$(SSH) "sudo insmod $(REMOTE_BUILD)/$*/$*.ko"
	@echo "recargado: $*"

dmesg-%:
	@$(SSH) "sudo dmesg | tail -n $(DMESG_LINES) | grep -i $* || echo '(sin entradas de $* en dmesg)'"

# ---------------------------------------------------------------------
# Clientes: src/<nombre>/main.c (programa de usuario que usa /dev/<nombre>)
# Se compila y corre ADENTRO de la VM.
#
#   make cliente-nulo            # solo compila main.c
#   make exec-nulo ARGS="a b"    # compila y corre (el driver ya debe estar cargado)
#   make run-nulo ARGS="a b"     # recarga el driver, compila y corre el cliente
#
# Para más de un archivo: make run-nulo CLIENT_SRCS="main.c util.c"
# ---------------------------------------------------------------------

CLIENT_SRCS    ?= main.c
CFLAGS_CLIENTE ?= -Wall -Wextra -g
ARGS           ?=

define COMPILAR_CLIENTE
@test -f $(SRC_DIR)/$(1)/main.c || { echo "Falta $(SRC_DIR)/$(1)/main.c"; exit 1; }
@$(SSH) "mkdir -p $(REMOTE_BUILD)/$(1)"
@tar -C $(SRC_DIR)/$(1) -c . | $(SSH) "tar -C $(REMOTE_BUILD)/$(1) -x"
@$(SSH) "cd $(REMOTE_BUILD)/$(1) && gcc $(CFLAGS_CLIENTE) -o main $(CLIENT_SRCS)"
@echo "cliente compilado (adentro de la VM): $(REMOTE_BUILD)/$(1)/main"
endef

cliente-%: start-vm
	$(call COMPILAR_CLIENTE,$*)

exec-%: cliente-%
	@$(SSH) "cd $(REMOTE_BUILD)/$* && ./main $(ARGS)"

run-%: reload-%
	$(call COMPILAR_CLIENTE,$*)
	@$(SSH) "cd $(REMOTE_BUILD)/$* && ./main $(ARGS)"

# ---------------------------------------------------------------------
# Limpieza
# ---------------------------------------------------------------------

# Descarga módulos y borra lo compilado dentro de la VM
clean:
	-@for m in $$(ls $(SRC_DIR) 2>/dev/null); do \
	    $(SSH) "sudo rmmod $$m 2>/dev/null"; done
	-@$(SSH) "rm -rf $(REMOTE_BUILD)"

# Borra TODO (VM, disco, claves). La imagen base se conserva.
distclean: stop-vm
	rm -f $(DISK) $(SEED) $(KEY) $(KEY).pub $(PIDFILE) $(LOG) \
	      $(VM_DIR)/user-data $(VM_DIR)/meta-data