# Stardew Valley: Gifts Needed

## Goal

Determine the fewest number of loved gifts needed for all Stardew Valley villagers, using set-covering

## Background

[Stardew Valley](https://www.stardewvalley.net/) is a terrific video game. In it, you can become friends with [various NPC villagers](https://stardewvalleywiki.com/Villagers) by giving them [gifts](https://stardewvalleywiki.com/Friendship#Gifts). Each villager "loves" different items, so the problem becomes to carry the fewest items with you to give them whenever you happen to come across each one. This becomes a [set-covering problem](https://en.m.wikipedia.org/wiki/Set_cover_problem).

Wiki: https://stardewvalleywiki.com/Stardew_Valley_Wiki

## General Info

All code is written by the repository owner, Laura Galbraith.

C++ style guidelines based on https://developers.google.com/edu/c++/getting-started?authuser=0

Version of C++ used: 2014 standard (c++ 14)

## Requirements

This code has only been built and run on an Ubuntu 20.04 environment, and is not officially supported outside of that environment.

### Required packages/programs

- make
- g++
- libcurl4-openssl-dev
- libxml2-dev

## Program Options

```
./determine_gifts.out [--skip-villagers "Villager1,Villager2"] [--missing-gifts "GiftA,GiftB"] [--help]
```

- `--skip-villagers` allows you to specify one or more villagers to not consider for gifting, in a comma-separated list
  - This can be useful if you've already reached [maximum hearts](https://stardewvalleywiki.com/Friendship#Point_system) with that villager.
- `--missing-gifts` allows you to specify any items that you do not have available to give out as gifts, in a comma-separated list
  - Many items, especially the [universally-loved gifts](https://stardewvalleywiki.com/Friendship#Universal_Loves), are hard to obtain in great quantities, or at all. You may also want to hold on to any number of them you already have.
- `--help` prints out the program usage, then exits
