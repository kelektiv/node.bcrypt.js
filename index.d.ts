// Type definitions for bcrypt 3.0
// Project: https://github.com/kelektiv/node.bcrypt.js/
// Definitions by: Andrey Kamozin <https://github.com/jaffparker>
// TypeScript Version: 2.6

export function genSaltSync(rounds?: number, minor?: 'a' | 'b'): string;

export function genSalt(rounds: number, minor: 'a' | 'b', callback: () => void): void;
export function genSalt(rounds: number, callback: (error: Error, salt: string) => void): void;
export function genSalt(callback: (error: Error, salt: string) => void): void;
export function genSalt(rounds?: number, minor?: 'a' | 'b'): Promise<string>;

export function hashSync(data: string, salt: string | number): string;

export function hash(data: string, salt: string, callback: (error: Error, hash: string) => void): void;
export function hash(data: string, salt: string): Promise<string>;
export function hash(data: string, saltRounds: number, callback: (error: Error, hash: string) => void): void;
export function hash(data: string, saltRounds: number): Promise<string>;

export function compareSync(data: string, hash: string): boolean;

export function compare(data: string, hash: string, callback: (error: Error, result: boolean) => void): void;
export function compare(data: string, hash: string): Promise<boolean>;

export function getRounds(hash: string): number
