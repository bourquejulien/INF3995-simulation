# INF3995 - Simulation

Ce repos inclus la simulation Argos.

L'exécution de la simulation nécéssite d'avoir argos3, webviz et GRPC d'installés. Comme il est long d'installer toutes ses dépendances, l'utilisation de docker est préférable.   

## Comment démarrer la simulation

### Prérequis
- docker 
- docker compose

### Procédure
1. Aller à la racine du répertoire cloné.
2. Exécuter la commande ``docker build -t simulation .``
3. Lancer la simulation avec ``docker run -p 8000:8000 -p 5500:5500 -p 9854:9854 -p 9855:9855 sim``
4. Se connecter à l'interface web de simulation à localhost:8000
