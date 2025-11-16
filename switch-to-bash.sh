#!/usr/bin/env bash
set -euo pipefail

echo ">>> Criando backup das configurações de zsh e p10k..."

BACKUP_DIR="$HOME/zsh-backup-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$BACKUP_DIR"

for f in .zshrc .p10k.zsh .oh-my-zsh; do
  if [ -e "$HOME/$f" ]; then
    echo "   - Movendo ~/$f -> $BACKUP_DIR/"
    mv "$HOME/$f" "$BACKUP_DIR"/
  fi
done

echo ">>> Criando .zshrc mínimo só para avisar da migração..."
cat > "$HOME/.zshrc" << 'EOF'
echo "Seu shell padrão agora é bash. O zsh foi desativado."
echo "Backup antigo em ~/zsh-backup-*"
EOF

echo ">>> Verificando se /bin/bash está em /etc/shells..."
if ! grep -q "/bin/bash" /etc/shells; then
  echo "ERRO: /bin/bash não está em /etc/shells. Abortando."
  exit 1
fi

echo ">>> Alterando shell padrão para /bin/bash (vai pedir sua senha)..."
chsh -s /bin/bash "$USER"

echo ">>> Garantindo configuração básica para o bash..."

if [ ! -f "$HOME/.bash_profile" ] && [ ! -f "$HOME/.bash_login" ] && [ ! -f "$HOME/.profile" ]; then
  cat > "$HOME/.bash_profile" << 'EOF'
# Bash profile simples
[[ -f ~/.bashrc ]] && . ~/.bashrc
EOF
  echo "   - Criado ~/.bash_profile"
fi

if [ ! -f "$HOME/.bashrc" ]; then
  cat > "$HOME/.bashrc" << 'EOF'
# Prompt simples no bash
PS1='\u@\h \w \$ '
EOF
  echo "   - Criado ~/.bashrc"
fi

echo
echo ">>> Concluído!"
echo "Backup das configs de zsh/p10k: $BACKUP_DIR"
echo "Feche todas as janelas do Terminal e abra de novo."
echo "O shell padrão agora deve ser o bash (verifique com: echo \$SHELL)."
