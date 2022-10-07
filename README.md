# INF3995 - Simulation

Ce repos inclus la simulation Argos.

L'exécution de la simulation nécessite d'avoir argos3, webviz et GRPC d'installés. Comme il est long d'installer toutes ces dépendances, on utilise un docker où elles sont déja là.

## Comment démarrer la simulation

### Prérequis
- docker 
- docker compose

### Procédure
1. Aller à la racine du répertoire cloné.
2. Exécuter la commande ``sudo docker build -t sim .`` pour compiler la simulation après modifications dans l'image "sim"
3. Lancer l'image avec ``sudo docker run -p 3000:3000 -p 8000:8000 -p 9854:9854 -p 9855:9855 -it sim``
4. Dans un browser, connectez vous à localhost:8000 pour voir l'interface visuelle de la simulation. Pour faire voler les drones, lancer aussi le backend

## Code

Le code est divisé en deux parties principales: experiments et main_simulation. Experiments définie les paramètres de l'environement physique de la simulation: les murs de l'arène, les drones utilisés, et la quantité de drones, entre autres. 

La classe CMainSimulation définie la logique du drone, avec ce qui se passe quand il reçoit des commandes spécifiques. 

Il y a aussi un repertoir communication qui met en place l'interface pour communiquer avec la simulation à distance. 