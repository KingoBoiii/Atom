﻿using Atom;

namespace Sandbox.TopDown2DTest
{

    internal class Player : Entity
    {

        [VisibleInEditor] private float m_Speed = 1.0f;
        [VisibleInEditor] private float m_Time = 0.0f;

        private Rigidbody2D rb2d;

        protected override void Start()
        {
            rb2d = GetComponent<Rigidbody2D>();

            var collider = GetComponent<BoxCollider2D>();
            collider.AddOnCollision2DEnterCallback((entity) =>
            {
                Scene.DestroyEntity(entity);
            });

            Entity enemy = Scene.CreateEntity("Enemy");
            var renderer = enemy.AddComponent<BasicRenderer>();
            renderer.Color = new Color(1.0f, 0.0f, 1.0f, 1.0f);

#if true
            var enemyCollider = enemy.AddComponent<BoxCollider2D>();
            //enemyCollider.AddOnCollision2DEnterCallback((entity) =>
            //{
            //    Log.Info("kgjfdogd");
            //    Scene.DestroyEntity(entity);
            //});

            var enemyrb2d = enemy.AddComponent<Rigidbody2D>();
            enemyrb2d.BodyType = RigidbodyType.Dynamic;
#endif
        }

        protected override void Update(float deltaTime)
        {
            m_Time += deltaTime;
            //base.Update(deltaTime);

            //Log.Warn("Player.Update(DeltaTime: {0}): Speed: {1}", deltaTime, m_Speed);

            var velocity = new Vector2(0.0f, 0.0f);

            if (Input.IsKeyDown(KeyCode.W))
            {
                velocity.Y += m_Speed / deltaTime;
            }
            else if (Input.IsKeyDown(KeyCode.S))
            {
                velocity.Y -= m_Speed / deltaTime;
            }

            if (Input.IsKeyDown(KeyCode.A))
            {
                velocity.X -= m_Speed / deltaTime;
            }
            else if (Input.IsKeyDown(KeyCode.D))
            {
                velocity.X += m_Speed / deltaTime;
            }

            rb2d.SetLinearVelocity(velocity);
        }

    }

}
