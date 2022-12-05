

# INF3995 - Simulation

Ce répertoire inclut la simulation Argos.

L'exécution de la simulation nécessite d'avoir installé argos3, webviz et gRPC. Comme il est long d'installer toutes ces dépendances, l'utilisation d'un docker où elles sont déjà installées est préférable.

## Comment démarrer la simulation

### Prérequis
- docker 
- docker compose

### Procédure
1. Aller à la racine du répertoire cloné.
2. Exécuter la commande ```docker build --target=final -t sim .``` pour compiler la simulation après modifications dans l'image "sim".
3. Lancer l'image avec ```docker run -p 3000:3000 -p 8000:8000 -p 9850-9860:9850-9860 -t sim``` (ou juste ```docker run -p 3000:3000 -p 8000:8000 -t sim``` s'il n'est pas nécessaire de se connecter avec le serveur).
4. Dans un navigateur, se connecter à localhost:8000 pour voir l'interface visuelle de la simulation. Pour faire voler les drones, lancer aussi le backend.

## Code

Le code est divisé en deux parties principales: experiments et main_simulation. Experiments définit les paramètres de l'environnement physique de la simulation: les murs de l'arène, les drones utilisés et la quantité de drones, entre autres.

La classe CMainSimulation définit la logique du drone, avec ce qu'il se passe quand il reçoit des commandes spécifiques.

Il y a aussi un répertoire "communication" qui met en place l'interface pour communiquer avec la simulation à distance.

## Formatage

Le formatage est exécuté à l'aide de [*clang-format*](https://clang.llvm.org/docs/ClangFormat.html) qui suit le
standard [**LLVM**](https://llvm.org/docs/CodingStandards.html).

Afin de valider le formatage, il est possible de lancer :

```bash
./format.sh
```

L'ajout de l'option ``-f`` permet de formater les fichiers.
